#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <inttypes.h>
#include "../Basic/basic.hpp"
#include "../File/file_utils.hpp"
#include "../Printf/printf.hpp"
#include "compatebility_stack_trace.hpp"

#if defined(_WIN32)
# include <windows.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
# include <execinfo.h>
#endif

#if defined(__linux__)
# include <dlfcn.h>
#endif

static void    cmp_stack_trace_strip_line_ending(char *line)
{
    ft_size_t    line_length;

    if (line == NULL)
        return ;
    line_length = ft_strlen_size_t(line);
    while (line_length > 0
        && (line[line_length - 1] == '\n'
            || line[line_length - 1] == '\r'))
    {
        line[line_length - 1] = '\0';
        line_length -= 1;
    }
    return ;
}

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

static int32_t    cmp_stack_trace_copy_text(char *destination,
        ft_size_t capacity, const char *source)
{
    int32_t    formatted_length;

    if (destination == NULL || source == NULL || capacity == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    formatted_length = pf_snprintf(destination, capacity, "%s", source);
    if (formatted_length < 0)
        return (FT_ERR_SYSTEM);
    if (static_cast<ft_size_t>(formatted_length) >= capacity)
        return (FT_ERR_OUT_OF_RANGE);
    return (FT_ERR_SUCCESS);
}

static int32_t    cmp_stack_trace_symbolize_linux(const void *address,
        char *symbol_buffer, ft_size_t symbol_buffer_size,
        char *location_buffer, ft_size_t location_buffer_size)
{
    Dl_info          info;
    uintptr_t        frame_address;
    uintptr_t        base_address;
    uintptr_t        relative_address;
    char             escaped_path[1024];
    char             command[1280];
    char             line_buffer[1024];
    FILE             *pipe_file;

    if (address == NULL || symbol_buffer == NULL || location_buffer == NULL
        || symbol_buffer_size == 0 || location_buffer_size == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    symbol_buffer[0] = '\0';
    location_buffer[0] = '\0';
    if (dladdr(address, &info) == 0 || info.dli_fname == NULL)
        return (FT_ERR_NOT_FOUND);
    frame_address = reinterpret_cast<uintptr_t>(address);
    base_address = reinterpret_cast<uintptr_t>(info.dli_fbase);
    if (frame_address >= base_address)
        relative_address = frame_address - base_address;
    else
        relative_address = frame_address;
    cmp_shell_escape_double_quotes(escaped_path, sizeof(escaped_path),
        info.dli_fname);
    if (pf_snprintf(command, sizeof(command),
        "addr2line -f -C -e \"%s\" 0x%" PRIxPTR " 2>/dev/null",
        escaped_path, relative_address) < 0)
        return (FT_ERR_SYSTEM);
    if (static_cast<ft_size_t>(ft_strlen_size_t(command)) >= sizeof(command))
        return (FT_ERR_SYSTEM);
    pipe_file = popen(command, "r");
    if (pipe_file == NULL)
        return (FT_ERR_SYSTEM);
    if (ft_fgets(line_buffer, sizeof(line_buffer), pipe_file) == NULL)
    {
        pclose(pipe_file);
        return (FT_ERR_NOT_FOUND);
    }
    cmp_stack_trace_strip_line_ending(line_buffer);
    if (cmp_stack_trace_copy_text(symbol_buffer, symbol_buffer_size,
            line_buffer) != FT_ERR_SUCCESS)
    {
        pclose(pipe_file);
        return (FT_ERR_OUT_OF_RANGE);
    }
    if (ft_fgets(line_buffer, sizeof(line_buffer), pipe_file) == NULL)
    {
        pclose(pipe_file);
        return (FT_ERR_NOT_FOUND);
    }
    cmp_stack_trace_strip_line_ending(line_buffer);
    if (cmp_stack_trace_copy_text(location_buffer, location_buffer_size,
            line_buffer) != FT_ERR_SUCCESS)
    {
        pclose(pipe_file);
        return (FT_ERR_OUT_OF_RANGE);
    }
    pclose(pipe_file);
    return (FT_ERR_SUCCESS);
}

static void    cmp_stack_trace_print_linux_addr2line(FILE *output_file,
        void *frame, ft_size_t frame_index)
{
    char             symbol_buffer[1024];
    char             location_buffer[1024];

    if (cmp_stack_trace_symbolize_address(frame, symbol_buffer,
            sizeof(symbol_buffer), location_buffer,
            sizeof(location_buffer)) != FT_ERR_SUCCESS)
    {
        ft_fprintf(output_file, "    #%zu %p\n", frame_index, frame);
        return ;
    }
    ft_fprintf(output_file, "    #%zu %s\n       %s\n", frame_index,
        symbol_buffer, location_buffer);
    return ;
}
#endif

#if defined(_WIN32)
static ft_size_t    cmp_stack_trace_capture_windows(void **frames,
        ft_size_t capacity, ft_size_t skip_count)
{
    USHORT    captured_count;
    ULONG     capture_capacity;

    if (frames == NULL || capacity == 0)
        return (0);
    if (capacity > static_cast<ft_size_t>(USHRT_MAX))
        capture_capacity = static_cast<ULONG>(USHRT_MAX);
    else
        capture_capacity = static_cast<ULONG>(capacity);
    if (skip_count > static_cast<ft_size_t>(ULONG_MAX))
        skip_count = static_cast<ft_size_t>(ULONG_MAX);
    captured_count = RtlCaptureStackBackTrace(static_cast<ULONG>(skip_count),
            capture_capacity, reinterpret_cast<PVOID *>(frames), NULL);
    return (static_cast<ft_size_t>(captured_count));
}

static int32_t    cmp_stack_trace_copy_windows_text(char *destination,
        ft_size_t capacity, const char *source)
{
    int32_t    formatted_length;

    if (destination == NULL || source == NULL || capacity == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    formatted_length = pf_snprintf(destination, capacity, "%s", source);
    if (formatted_length < 0)
        return (FT_ERR_SYSTEM);
    if (static_cast<ft_size_t>(formatted_length) >= capacity)
        return (FT_ERR_OUT_OF_RANGE);
    return (FT_ERR_SUCCESS);
}

static int32_t    cmp_stack_trace_run_addr2line_command(const char *command,
        char *symbol_buffer, ft_size_t symbol_buffer_size,
        char *location_buffer, ft_size_t location_buffer_size)
{
    char    line_buffer[1024];
    FILE    *pipe_file;

    pipe_file = popen(command, "r");
    if (pipe_file == NULL)
        return (FT_ERR_SYSTEM);
    if (fgets(line_buffer, sizeof(line_buffer), pipe_file) == NULL)
    {
        pclose(pipe_file);
        return (FT_ERR_NOT_FOUND);
    }
    cmp_stack_trace_strip_line_ending(line_buffer);
    if (cmp_stack_trace_copy_windows_text(symbol_buffer, symbol_buffer_size,
            line_buffer) != FT_ERR_SUCCESS)
    {
        pclose(pipe_file);
        return (FT_ERR_OUT_OF_RANGE);
    }
    if (fgets(line_buffer, sizeof(line_buffer), pipe_file) == NULL)
    {
        pclose(pipe_file);
        return (FT_ERR_NOT_FOUND);
    }
    cmp_stack_trace_strip_line_ending(line_buffer);
    if (cmp_stack_trace_copy_windows_text(location_buffer,
            location_buffer_size, line_buffer) != FT_ERR_SUCCESS)
    {
        pclose(pipe_file);
        return (FT_ERR_OUT_OF_RANGE);
    }
    pclose(pipe_file);
    return (FT_ERR_SUCCESS);
}

static int32_t    cmp_stack_trace_symbolize_windows(const void *address,
        char *symbol_buffer, ft_size_t symbol_buffer_size,
        char *location_buffer, ft_size_t location_buffer_size)
{
    HMODULE           module_handle;
    uintptr_t         frame_address;
    uintptr_t         module_base_address;
    uintptr_t         text_section_address;
    char              module_path[MAX_PATH];
    char              command[1280];
    int32_t           command_length;

    if (address == NULL || symbol_buffer == NULL || location_buffer == NULL
        || symbol_buffer_size == 0 || location_buffer_size == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    symbol_buffer[0] = '\0';
    location_buffer[0] = '\0';
    module_handle = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
            | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCSTR>(address), &module_handle) == FALSE)
        return (FT_ERR_NOT_FOUND);
    if (GetModuleFileNameA(module_handle, module_path, MAX_PATH) == 0)
        return (FT_ERR_NOT_FOUND);
    frame_address = reinterpret_cast<uintptr_t>(address);
    module_base_address = reinterpret_cast<uintptr_t>(module_handle);
    text_section_address = 0;
    {
        PIMAGE_DOS_HEADER    dos_header;
        PIMAGE_NT_HEADERS    nt_headers;
        PIMAGE_SECTION_HEADER    section_header;
        uint16_t             section_index;

        dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_base_address);
        if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
            return (FT_ERR_NOT_FOUND);
        nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(
                module_base_address + static_cast<uintptr_t>(dos_header->e_lfanew));
        if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
            return (FT_ERR_NOT_FOUND);
        section_header = reinterpret_cast<PIMAGE_SECTION_HEADER>(
                reinterpret_cast<unsigned char *>(nt_headers)
                + sizeof(DWORD)
                + sizeof(IMAGE_FILE_HEADER)
                + static_cast<ft_size_t>(
                    nt_headers->FileHeader.SizeOfOptionalHeader));
        section_index = 0;
        while (section_index < nt_headers->FileHeader.NumberOfSections)
        {
            if (section_header[section_index].Name[0] == '.'
                && section_header[section_index].Name[1] == 't'
                && section_header[section_index].Name[2] == 'e'
                && section_header[section_index].Name[3] == 'x'
                && section_header[section_index].Name[4] == 't'
                && section_header[section_index].Name[5] == '\0')
            {
                text_section_address = module_base_address
                    + static_cast<uintptr_t>(
                        section_header[section_index].VirtualAddress);
                break ;
            }
            section_index += 1;
        }
    }
    if (text_section_address == 0 || frame_address < text_section_address)
        return (FT_ERR_NOT_FOUND);
    command_length = pf_snprintf(command, sizeof(command),
            "addr2line -f -C -j .text -e \"%s\" 0x%" PRIxPTR " 2>nul",
            module_path, frame_address - text_section_address);
    if (command_length < 0
        || static_cast<ft_size_t>(command_length) >= sizeof(command))
        return (FT_ERR_SYSTEM);
    if (cmp_stack_trace_run_addr2line_command(command, symbol_buffer,
            symbol_buffer_size, location_buffer, location_buffer_size)
        == FT_ERR_SUCCESS)
        return (FT_ERR_SUCCESS);
    return (FT_ERR_NOT_FOUND);
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
#elif defined(_WIN32)
    return (cmp_stack_trace_capture_windows(frames, capacity, skip_count));
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
            ft_fprintf(output_file, "    #%" PRIu64 " %p\n",
                static_cast<uint64_t>(frame_index),
                frames[frame_index]);
            frame_index += 1;
        }
        return ;
    }
    frame_index = 0;
    while (frame_index < frame_count)
    {
        ft_fprintf(output_file, "    #%" PRIu64 " %s\n",
            static_cast<uint64_t>(frame_index),
            symbols[frame_index]);
        frame_index += 1;
    }
    free(symbols);
#elif defined(_WIN32)
    ft_size_t    frame_index;

    if (output_file == NULL || frames == NULL || frame_count == 0)
        return ;
    frame_index = 0;
    while (frame_index < frame_count)
    {
        ft_fprintf(output_file, "    #%" PRIu64 " %p\n",
            static_cast<uint64_t>(frame_index),
            frames[frame_index]);
        frame_index += 1;
    }
#else
    ft_size_t    frame_index;

    if (output_file == NULL || frames == NULL || frame_count == 0)
        return ;
    frame_index = 0;
    while (frame_index < frame_count)
    {
        ft_fprintf(output_file, "    #%" PRIu64 " %p\n",
            static_cast<uint64_t>(frame_index),
            frames[frame_index]);
        frame_index += 1;
    }
#endif
    return ;
}

int32_t    cmp_stack_trace_symbolize_address(const void *address,
        char *symbol_buffer, ft_size_t symbol_buffer_size,
        char *location_buffer, ft_size_t location_buffer_size)
{
#if defined(__linux__)
    if (address == NULL || symbol_buffer == NULL || location_buffer == NULL
        || symbol_buffer_size == 0 || location_buffer_size == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    return (cmp_stack_trace_symbolize_linux(address, symbol_buffer,
        symbol_buffer_size, location_buffer, location_buffer_size));
#elif defined(_WIN32)
    return (cmp_stack_trace_symbolize_windows(address, symbol_buffer,
        symbol_buffer_size, location_buffer, location_buffer_size));
#else
    (void)address;
    if (symbol_buffer != NULL && symbol_buffer_size > 0)
        symbol_buffer[0] = '\0';
    if (location_buffer != NULL && location_buffer_size > 0)
        location_buffer[0] = '\0';
    return (FT_ERR_INVALID_OPERATION);
#endif
}
