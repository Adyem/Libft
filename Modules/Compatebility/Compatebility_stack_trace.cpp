#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <inttypes.h>
#include "compatebility_stack_trace.hpp"

#if defined(__linux__) || defined(__APPLE__)
# include <execinfo.h>
#endif

#if defined(__linux__)
# include <dlfcn.h>
#endif

#if defined(__linux__)
static ft_size_t    cmp_shell_escape_double_quotes(char *destination,
        ft_size_t capacity, const char *source)
{
    ft_size_t    source_index;
    ft_size_t    destination_index;

    if (destination == NULL || capacity == 0 || source == NULL)
        return (0);
    source_index = 0;
    destination_index = 0;
    while (source[source_index] != '\0' && destination_index + 1 < capacity)
    {
        if ((source[source_index] == '"' || source[source_index] == '\\'
                || source[source_index] == '$'
                || source[source_index] == '`')
            && destination_index + 2 < capacity)
        {
            destination[destination_index] = '\\';
            destination_index += 1;
        }
        destination[destination_index] = source[source_index];
        destination_index += 1;
        source_index += 1;
    }
    destination[destination_index] = '\0';
    return (destination_index);
}

static void    cmp_stack_trace_print_linux_addr2line(FILE *output_file,
        void *frame, ft_size_t frame_index)
{
    Dl_info          info;
    uintptr_t        frame_address;
    uintptr_t        base_address;
    uintptr_t        relative_address;
    char             escaped_path[1024];
    char             command[1280];
    char             line_buffer[1024];
    FILE             *pipe_file;
    ft_bool          printed_detail;

    if (dladdr(frame, &info) == 0 || info.dli_fname == NULL)
    {
        std::fprintf(output_file, "    #%zu %p\n", frame_index, frame);
        return ;
    }
    frame_address = reinterpret_cast<uintptr_t>(frame);
    base_address = reinterpret_cast<uintptr_t>(info.dli_fbase);
    if (frame_address >= base_address)
        relative_address = frame_address - base_address;
    else
        relative_address = frame_address;
    cmp_shell_escape_double_quotes(escaped_path, sizeof(escaped_path),
        info.dli_fname);
    std::snprintf(command, sizeof(command),
        "addr2line -f -C -e \"%s\" %zx 2>/dev/null",
        escaped_path, relative_address);
    pipe_file = popen(command, "r");
    if (pipe_file == NULL)
    {
        std::fprintf(output_file, "    #%zu %p %s\n", frame_index, frame,
            info.dli_fname);
        return ;
    }
    printed_detail = FT_FALSE;
    while (std::fgets(line_buffer, sizeof(line_buffer), pipe_file) != NULL)
    {
        ft_size_t    line_length;

        line_length = std::strlen(line_buffer);
        while (line_length > 0
            && (line_buffer[line_length - 1] == '\n'
                || line_buffer[line_length - 1] == '\r'))
        {
            line_buffer[line_length - 1] = '\0';
            line_length -= 1;
        }
        if (printed_detail == FT_FALSE)
        {
            std::fprintf(output_file, "    #%zu %s\n", frame_index,
                line_buffer);
            printed_detail = FT_TRUE;
        }
        else
            std::fprintf(output_file, "       %s\n", line_buffer);
    }
    pclose(pipe_file);
    if (printed_detail == FT_FALSE)
    {
        std::fprintf(output_file, "    #%zu %p %s\n", frame_index, frame,
            info.dli_fname);
    }
    return ;
}
#endif

ft_size_t    cmp_stack_trace_capture(void **frames, ft_size_t capacity,
        ft_size_t skip_count)
{
#if defined(__linux__) || defined(__APPLE__)
    void    *captured_frames[CMP_STACK_TRACE_MAX_FRAMES + 8];
    ft_size_t    capture_capacity;
    int32_t    captured_count;
    ft_size_t    output_index;

    if (frames == NULL || capacity == 0)
        return (0);
    capture_capacity = capacity + skip_count;
    if (capture_capacity > CMP_STACK_TRACE_MAX_FRAMES + 8)
        capture_capacity = CMP_STACK_TRACE_MAX_FRAMES + 8;
    captured_count = backtrace(captured_frames, static_cast<int>(capture_capacity));
    if (captured_count <= 0)
        return (0);
    output_index = 0;
    while (static_cast<ft_size_t>(captured_count) > skip_count
        && output_index + skip_count < static_cast<ft_size_t>(captured_count)
        && output_index < capacity)
    {
        frames[output_index] = captured_frames[output_index + skip_count];
        output_index += 1;
    }
    return (output_index);
#else
    (void)frames;
    (void)capacity;
    (void)skip_count;
    return (0);
#endif
}

void    cmp_stack_trace_print(FILE *output_file, void *const *frames,
        ft_size_t frame_count)
{
#if defined(__linux__)
    ft_size_t    frame_index;

    if (output_file == NULL || frames == NULL || frame_count == 0)
        return ;
    frame_index = 0;
    while (frame_index < frame_count)
    {
        cmp_stack_trace_print_linux_addr2line(output_file, frames[frame_index],
            frame_index);
        frame_index += 1;
    }
#elif defined(__APPLE__)
    char    **symbols;
    ft_size_t    frame_index;

    if (output_file == NULL || frames == NULL || frame_count == 0)
        return ;
    symbols = backtrace_symbols(const_cast<void *const *>(frames),
            static_cast<int>(frame_count));
    if (symbols == NULL)
    {
        frame_index = 0;
        while (frame_index < frame_count)
        {
            std::fprintf(output_file, "    #%" PRIu64 " %p\n",
                static_cast<uint64_t>(frame_index),
                frames[frame_index]);
            frame_index += 1;
        }
        return ;
    }
    frame_index = 0;
    while (frame_index < frame_count)
    {
        std::fprintf(output_file, "    #%" PRIu64 " %s\n",
            static_cast<uint64_t>(frame_index),
            symbols[frame_index]);
        frame_index += 1;
    }
    std::free(symbols);
#else
    ft_size_t    frame_index;

    if (output_file == NULL || frames == NULL || frame_count == 0)
        return ;
    frame_index = 0;
    while (frame_index < frame_count)
    {
        std::fprintf(output_file, "    #%zu %p\n", frame_index,
            frames[frame_index]);
        frame_index += 1;
    }
#endif
    return ;
}
