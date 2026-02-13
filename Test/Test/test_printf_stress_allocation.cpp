#include "../test_internal.hpp"
#include "../../Printf/printf.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

typedef int (*t_pf_printf_fd_plain)(int, const char *, ...);
typedef int (*t_pf_snprintf_plain)(char *, size_t, const char *, ...);
typedef int (*t_ft_fprintf_plain)(FILE *, const char *, ...);

struct t_pf_stress_context
{
    const char  *chunk;
    int         repeat_count;
};

static int pf_vsnprintf_wrapper_stress(char *string, size_t size,
    const char *format, ...)
{
    va_list arguments;
    int result;

    va_start(arguments, format);
    result = pf_vsnprintf(string, size, format, arguments);
    va_end(arguments);
    return (result);
}

static void pf_fill_repeated(char *buffer, size_t length, char value)
{
    size_t index;

    index = 0;
    while (index < length)
    {
        buffer[index] = value;
        index += 1;
    }
    buffer[length] = '\0';
    return ;
}

static ft_size_t pf_current_cma_bytes(void)
{
    ft_size_t allocation_count;
    ft_size_t free_count;
    ft_size_t current_bytes;
    ft_size_t peak_bytes;
    int32_t status;

    allocation_count = 0;
    free_count = 0;
    current_bytes = 0;
    peak_bytes = 0;
    status = cma_get_extended_stats(&allocation_count, &free_count,
            &current_bytes, &peak_bytes);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, status);
    return (current_bytes);
}

static int pf_read_file(const char *file_name, char *buffer, size_t buffer_size)
{
    int file_descriptor;
    ssize_t read_size;

    if (file_name == ft_nullptr || buffer == ft_nullptr || buffer_size == 0)
        return (0);
    file_descriptor = open(file_name, O_RDONLY);
    if (file_descriptor < 0)
        return (0);
    read_size = read(file_descriptor, buffer, buffer_size - 1);
    close(file_descriptor);
    if (read_size < 0)
        return (0);
    buffer[static_cast<size_t>(read_size)] = '\0';
    return (1);
}

static int pf_custom_stress_handler(va_list *arguments, ft_string &output,
    void *context)
{
    t_pf_stress_context *stress_context;
    int index;

    (void)arguments;
    stress_context = static_cast<t_pf_stress_context *>(context);
    if (stress_context == ft_nullptr || stress_context->chunk == ft_nullptr)
        return (-1);
    output.clear();
    index = 0;
    while (index < stress_context->repeat_count)
    {
        if (output.append(stress_context->chunk) != FT_ERR_SUCCESS)
            return (-1);
        index += 1;
    }
    return (0);
}

static int pf_custom_echo_handler(va_list *arguments, ft_string &output,
    void *context)
{
    const char *prefix;
    const char *value;

    prefix = static_cast<const char *>(context);
    if (arguments == ft_nullptr)
        return (-1);
    value = va_arg(*arguments, const char *);
    output.clear();
    if (prefix != ft_nullptr)
    {
        if (output.append(prefix) != FT_ERR_SUCCESS)
            return (-1);
    }
    if (value == ft_nullptr)
    {
        if (output.append("(null)") != FT_ERR_SUCCESS)
            return (-1);
        return (0);
    }
    if (output.append(value) != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

static int pf_run_stress_snprintf_with_limit(ft_size_t limit, char specifier,
    const char *value)
{
    char format[3];
    char buffer[64];
    int result;

    format[0] = '%';
    format[1] = specifier;
    format[2] = '\0';
    cma_set_alloc_limit(limit);
    result = pf_snprintf(buffer, sizeof(buffer), format, value);
    cma_set_alloc_limit(0);
    return (result);
}

FT_TEST(test_printf_stress_long_string_exact_fit,
    "pf_snprintf handles long complex strings with exact-fit destination")
{
    char large_input[1025];
    char output_buffer[1100];
    int printed;

    pf_fill_repeated(large_input, 1024, 'x');
    printed = pf_snprintf(output_buffer, sizeof(output_buffer),
            "prefix:%s:suffix", large_input);
    FT_ASSERT_EQ(1038, printed);
    FT_ASSERT_EQ(0, ft_strncmp(output_buffer, "prefix:xxx", 10));
    FT_ASSERT_EQ(0, ft_strcmp(&output_buffer[1031], ":suffix"));
    return (1);
}

FT_TEST(test_printf_stress_long_string_truncation,
    "pf_snprintf truncates complex long strings and keeps terminator")
{
    char large_input[2049];
    char output_buffer[33];
    int printed;

    pf_fill_repeated(large_input, 2048, 'y');
    printed = pf_snprintf(output_buffer, sizeof(output_buffer), "[%s]", large_input);
    FT_ASSERT_EQ(2050, printed);
    FT_ASSERT_EQ('[', output_buffer[0]);
    FT_ASSERT_EQ('\0', output_buffer[32]);
    return (1);
}

FT_TEST(test_printf_stress_mixed_numeric_formats,
    "pf_snprintf handles mixed numeric formatting under high complexity")
{
    char output_buffer[256];
    int printed;

    printed = pf_snprintf(output_buffer, sizeof(output_buffer),
            "%ld|%lu|%lx|%zu|%zx|%.4f|%c|%s",
            2147483648L, 2147483648UL, 2147483648UL,
            static_cast<size_t>(4294967295ULL),
            static_cast<size_t>(4294967295ULL), 3.1415926, 'Z', "tail");
    FT_ASSERT(printed > 0);
    FT_ASSERT(ft_strstr(output_buffer, "80000000") != ft_nullptr);
    FT_ASSERT(ft_strstr(output_buffer, "3.1416") != ft_nullptr);
    FT_ASSERT(ft_strstr(output_buffer, "tail") != ft_nullptr);
    return (1);
}

FT_TEST(test_printf_stress_printf_fd_large_payload,
    "pf_printf_fd writes large payloads to files correctly")
{
    const char *file_name;
    int file_descriptor;
    char input_data[901];
    char read_buffer[1024];
    int printed;

    file_name = "tmp_test_printf_stress_fd.txt";
    pf_fill_repeated(input_data, 900, 'k');
    file_descriptor = open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (0);
    printed = pf_printf_fd(file_descriptor, "<%s>", input_data);
    close(file_descriptor);
    FT_ASSERT_EQ(902, printed);
    FT_ASSERT(pf_read_file(file_name, read_buffer, sizeof(read_buffer)));
    unlink(file_name);
    FT_ASSERT_EQ('<', read_buffer[0]);
    FT_ASSERT_EQ('>', read_buffer[901]);
    return (1);
}

FT_TEST(test_printf_stress_fprintf_large_payload,
    "ft_fprintf writes large payloads to regular streams")
{
    const char *file_name;
    FILE *stream;
    char input_data[1201];
    char read_buffer[1400];
    int printed;

    file_name = "tmp_test_printf_stress_stream.txt";
    pf_fill_repeated(input_data, 1200, 'p');
    stream = fopen(file_name, "w+");
    if (stream == ft_nullptr)
        return (0);
    printed = ft_fprintf(stream, "(%s)", input_data);
    fflush(stream);
    fclose(stream);
    FT_ASSERT_EQ(1202, printed);
    FT_ASSERT(pf_read_file(file_name, read_buffer, sizeof(read_buffer)));
    unlink(file_name);
    FT_ASSERT_EQ('(', read_buffer[0]);
    FT_ASSERT_EQ(')', read_buffer[1201]);
    return (1);
}

FT_TEST(test_printf_stress_custom_specifier_large_success,
    "custom specifier can build large payloads successfully")
{
    char chunk[257];
    char format[3];
    char output_buffer[5000];
    t_pf_stress_context context;
    t_pf_snprintf_plain call_pf_snprintf;
    int printed;

    pf_fill_repeated(chunk, 256, 'q');
    context.chunk = chunk;
    context.repeat_count = 10;
    format[0] = '%';
    format[1] = 'J';
    format[2] = '\0';
    pf_unregister_custom_specifier('J');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('J', pf_custom_stress_handler,
            &context));
    call_pf_snprintf = pf_snprintf;
    printed = call_pf_snprintf(output_buffer, sizeof(output_buffer), format);
    FT_ASSERT_EQ(2560, printed);
    FT_ASSERT_EQ('q', output_buffer[0]);
    FT_ASSERT_EQ('q', output_buffer[2559]);
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('J'));
    return (1);
}

FT_TEST(test_printf_stress_custom_echo_null_input,
    "custom echo specifier handles null argument in complex path")
{
    char format[3];
    char output_buffer[64];
    int printed;

    format[0] = '%';
    format[1] = 'K';
    format[2] = '\0';
    pf_unregister_custom_specifier('K');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('K', pf_custom_echo_handler,
            (void *)"prefix:"));
    printed = pf_snprintf(output_buffer, sizeof(output_buffer), format,
            static_cast<const char *>(ft_nullptr));
    FT_ASSERT_EQ(13, printed);
    FT_ASSERT_EQ(0, ft_strcmp(output_buffer, "prefix:(null)"));
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('K'));
    return (1);
}

FT_TEST(test_printf_stress_thread_safety_toggle_and_format,
    "custom formatting still works after thread-safety toggles")
{
    char format[3];
    char output_buffer[64];

    format[0] = '%';
    format[1] = 'L';
    format[2] = '\0';
    pf_disable_thread_safety();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pf_enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pf_enable_thread_safety());
    pf_unregister_custom_specifier('L');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('L', pf_custom_echo_handler,
            (void *)"id:"));
    FT_ASSERT_EQ(6, pf_snprintf(output_buffer, sizeof(output_buffer), format, "42"));
    FT_ASSERT_EQ(0, ft_strcmp(output_buffer, "id:42"));
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('L'));
    pf_disable_thread_safety();
    return (1);
}

FT_TEST(test_printf_stress_vsnprintf_large_payload,
    "pf_vsnprintf wrapper handles long mixed payloads")
{
    char input_data[701];
    char output_buffer[800];
    int printed;

    pf_fill_repeated(input_data, 700, 'm');
    printed = pf_vsnprintf_wrapper_stress(output_buffer, sizeof(output_buffer),
            "A:%s:B:%d", input_data, 9);
    FT_ASSERT_EQ(706, printed);
    FT_ASSERT_EQ('A', output_buffer[0]);
    FT_ASSERT_EQ(':', output_buffer[1]);
    FT_ASSERT_EQ('9', output_buffer[705]);
    return (1);
}

FT_TEST(test_printf_alloc_limit_snprintf_failure_limit_1,
    "pf_snprintf fails with custom allocator limit 1 in stress handler")
{
    char chunk[257];
    t_pf_stress_context context;

    pf_fill_repeated(chunk, 256, 'a');
    context.chunk = chunk;
    context.repeat_count = 12;
    pf_unregister_custom_specifier('M');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('M', pf_custom_stress_handler,
            &context));
    FT_ASSERT_EQ(-1, pf_run_stress_snprintf_with_limit(1, 'M', "x"));
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('M'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_snprintf_failure_limit_2,
    "pf_snprintf fails with custom allocator limit 2 in stress handler")
{
    char chunk[257];
    t_pf_stress_context context;

    pf_fill_repeated(chunk, 256, 'b');
    context.chunk = chunk;
    context.repeat_count = 12;
    pf_unregister_custom_specifier('N');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('N', pf_custom_stress_handler,
            &context));
    FT_ASSERT_EQ(-1, pf_run_stress_snprintf_with_limit(2, 'N', "x"));
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('N'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_snprintf_failure_limit_3,
    "pf_snprintf fails with custom allocator limit 3 in stress handler")
{
    char chunk[257];
    t_pf_stress_context context;

    pf_fill_repeated(chunk, 256, 'c');
    context.chunk = chunk;
    context.repeat_count = 14;
    pf_unregister_custom_specifier('O');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('O', pf_custom_stress_handler,
            &context));
    FT_ASSERT_EQ(-1, pf_run_stress_snprintf_with_limit(3, 'O', "x"));
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('O'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_snprintf_failure_limit_4,
    "pf_snprintf fails with custom allocator limit 4 in stress handler")
{
    char chunk[257];
    t_pf_stress_context context;

    pf_fill_repeated(chunk, 256, 'd');
    context.chunk = chunk;
    context.repeat_count = 16;
    pf_unregister_custom_specifier('P');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('P', pf_custom_stress_handler,
            &context));
    FT_ASSERT_EQ(-1, pf_run_stress_snprintf_with_limit(4, 'P', "x"));
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('P'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_snprintf_failure_limit_5,
    "pf_snprintf fails with custom allocator limit 5 in stress handler")
{
    char chunk[257];
    t_pf_stress_context context;

    pf_fill_repeated(chunk, 256, 'e');
    context.chunk = chunk;
    context.repeat_count = 18;
    pf_unregister_custom_specifier('R');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('R', pf_custom_stress_handler,
            &context));
    FT_ASSERT_EQ(-1, pf_run_stress_snprintf_with_limit(5, 'R', "x"));
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('R'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_vsnprintf_failure,
    "pf_vsnprintf fails under allocation limit in stress handler")
{
    char chunk[257];
    char format[3];
    char output_buffer[64];
    t_pf_stress_context context;
    int result;

    pf_fill_repeated(chunk, 256, 'f');
    context.chunk = chunk;
    context.repeat_count = 16;
    format[0] = '%';
    format[1] = 'S';
    format[2] = '\0';
    pf_unregister_custom_specifier('S');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('S', pf_custom_stress_handler,
            &context));
    cma_set_alloc_limit(3);
    result = pf_vsnprintf_wrapper_stress(output_buffer, sizeof(output_buffer),
            format);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('S'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_printf_fd_failure,
    "pf_printf_fd fails under allocation limit in stress handler")
{
    char chunk[257];
    char format[3];
    const char *file_name;
    int file_descriptor;
    t_pf_stress_context context;
    t_pf_printf_fd_plain call_pf_printf_fd;
    int result;

    pf_fill_repeated(chunk, 256, 'g');
    context.chunk = chunk;
    context.repeat_count = 16;
    format[0] = '%';
    format[1] = 'T';
    format[2] = '\0';
    file_name = "tmp_test_printf_alloc_fd.txt";
    file_descriptor = open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (0);
    pf_unregister_custom_specifier('T');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('T', pf_custom_stress_handler,
            &context));
    call_pf_printf_fd = pf_printf_fd;
    cma_set_alloc_limit(3);
    result = call_pf_printf_fd(file_descriptor, format);
    cma_set_alloc_limit(0);
    close(file_descriptor);
    unlink(file_name);
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('T'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_fprintf_failure,
    "ft_fprintf fails under allocation limit in stress handler")
{
    char chunk[257];
    char format[3];
    FILE *stream;
    t_pf_stress_context context;
    t_ft_fprintf_plain call_ft_fprintf;
    int result;

    pf_fill_repeated(chunk, 256, 'h');
    context.chunk = chunk;
    context.repeat_count = 16;
    format[0] = '%';
    format[1] = 'U';
    format[2] = '\0';
    stream = tmpfile();
    if (stream == ft_nullptr)
        return (0);
    pf_unregister_custom_specifier('U');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('U', pf_custom_stress_handler,
            &context));
    call_ft_fprintf = ft_fprintf;
    cma_set_alloc_limit(3);
    result = call_ft_fprintf(stream, format);
    cma_set_alloc_limit(0);
    fclose(stream);
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('U'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_failure_then_success_after_reset,
    "allocation-limited failure does not poison next successful formatting")
{
    char chunk[257];
    char format[3];
    char output_buffer[6000];
    t_pf_stress_context context;
    t_pf_snprintf_plain call_pf_snprintf;

    pf_fill_repeated(chunk, 256, 'i');
    context.chunk = chunk;
    context.repeat_count = 10;
    format[0] = '%';
    format[1] = 'V';
    format[2] = '\0';
    pf_unregister_custom_specifier('V');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('V', pf_custom_stress_handler,
            &context));
    call_pf_snprintf = pf_snprintf;
    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(-1, call_pf_snprintf(output_buffer, sizeof(output_buffer), format));
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(2560, call_pf_snprintf(output_buffer, sizeof(output_buffer), format));
    FT_ASSERT_EQ('i', output_buffer[0]);
    FT_ASSERT_EQ('i', output_buffer[2559]);
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('V'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_failure_no_leak_snprintf,
    "failed pf_snprintf under alloc limit keeps CMA bytes balanced")
{
    char chunk[257];
    char format[3];
    char output_buffer[64];
    t_pf_stress_context context;
    t_pf_snprintf_plain call_pf_snprintf;
    ft_size_t bytes_before;
    ft_size_t bytes_after;

    pf_fill_repeated(chunk, 256, 'j');
    context.chunk = chunk;
    context.repeat_count = 18;
    format[0] = '%';
    format[1] = 'W';
    format[2] = '\0';
    pf_unregister_custom_specifier('W');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('W', pf_custom_stress_handler,
            &context));
    call_pf_snprintf = pf_snprintf;
    bytes_before = pf_current_cma_bytes();
    cma_set_alloc_limit(2);
    FT_ASSERT_EQ(-1, call_pf_snprintf(output_buffer, sizeof(output_buffer), format));
    cma_set_alloc_limit(0);
    bytes_after = pf_current_cma_bytes();
    FT_ASSERT_EQ(bytes_before, bytes_after);
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('W'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_failure_no_leak_printf_fd,
    "failed pf_printf_fd under alloc limit keeps CMA bytes balanced")
{
    char chunk[257];
    char format[3];
    const char *file_name;
    int file_descriptor;
    t_pf_stress_context context;
    t_pf_printf_fd_plain call_pf_printf_fd;
    ft_size_t bytes_before;
    ft_size_t bytes_after;

    pf_fill_repeated(chunk, 256, 'l');
    context.chunk = chunk;
    context.repeat_count = 18;
    format[0] = '%';
    format[1] = 'X';
    format[2] = '\0';
    file_name = "tmp_test_printf_alloc_fd_leak.txt";
    file_descriptor = open(file_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (0);
    pf_unregister_custom_specifier('X');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('X', pf_custom_stress_handler,
            &context));
    call_pf_printf_fd = pf_printf_fd;
    bytes_before = pf_current_cma_bytes();
    cma_set_alloc_limit(2);
    FT_ASSERT_EQ(-1, call_pf_printf_fd(file_descriptor, format));
    cma_set_alloc_limit(0);
    bytes_after = pf_current_cma_bytes();
    close(file_descriptor);
    unlink(file_name);
    FT_ASSERT_EQ(bytes_before, bytes_after);
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('X'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_failure_no_leak_fprintf,
    "failed ft_fprintf under alloc limit keeps CMA bytes balanced")
{
    char chunk[257];
    char format[3];
    FILE *stream;
    t_pf_stress_context context;
    t_ft_fprintf_plain call_ft_fprintf;
    ft_size_t bytes_before;
    ft_size_t bytes_after;

    pf_fill_repeated(chunk, 256, 'n');
    context.chunk = chunk;
    context.repeat_count = 18;
    format[0] = '%';
    format[1] = 'Y';
    format[2] = '\0';
    stream = tmpfile();
    if (stream == ft_nullptr)
        return (0);
    pf_unregister_custom_specifier('Y');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('Y', pf_custom_stress_handler,
            &context));
    call_ft_fprintf = ft_fprintf;
    bytes_before = pf_current_cma_bytes();
    cma_set_alloc_limit(2);
    FT_ASSERT_EQ(-1, call_ft_fprintf(stream, format));
    cma_set_alloc_limit(0);
    bytes_after = pf_current_cma_bytes();
    fclose(stream);
    FT_ASSERT_EQ(bytes_before, bytes_after);
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('Y'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_with_thread_safety_enabled,
    "alloc-limit failure path works while custom formatter mutex is enabled")
{
    char chunk[257];
    char format[3];
    char output_buffer[64];
    t_pf_stress_context context;
    t_pf_snprintf_plain call_pf_snprintf;

    pf_fill_repeated(chunk, 256, 'o');
    context.chunk = chunk;
    context.repeat_count = 18;
    format[0] = '%';
    format[1] = 'Z';
    format[2] = '\0';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pf_enable_thread_safety());
    pf_unregister_custom_specifier('Z');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('Z', pf_custom_stress_handler,
            &context));
    call_pf_snprintf = pf_snprintf;
    cma_set_alloc_limit(2);
    FT_ASSERT_EQ(-1, call_pf_snprintf(output_buffer, sizeof(output_buffer), format));
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('Z'));
    pf_disable_thread_safety();
    return (1);
}

FT_TEST(test_printf_alloc_limit_repeated_failures_do_not_stale_state,
    "repeated alloc-limit failures keep custom formatter registry usable")
{
    char chunk[257];
    char format[3];
    char output_buffer[64];
    t_pf_stress_context context;
    t_pf_snprintf_plain call_pf_snprintf;
    int index;

    pf_fill_repeated(chunk, 256, 'r');
    context.chunk = chunk;
    context.repeat_count = 18;
    format[0] = '%';
    format[1] = 'a';
    format[2] = '\0';
    pf_unregister_custom_specifier('a');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('a', pf_custom_stress_handler,
            &context));
    call_pf_snprintf = pf_snprintf;
    index = 0;
    while (index < 3)
    {
        cma_set_alloc_limit(2);
        FT_ASSERT_EQ(-1, call_pf_snprintf(output_buffer, sizeof(output_buffer), format));
        cma_set_alloc_limit(0);
        index += 1;
    }
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('a'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_deeper_failure_limit_6,
    "deeper alloc-limit failure with limit 6 still returns formatting error")
{
    char chunk[257];
    t_pf_stress_context context;

    pf_fill_repeated(chunk, 256, 's');
    context.chunk = chunk;
    context.repeat_count = 28;
    pf_unregister_custom_specifier('b');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('b', pf_custom_stress_handler,
            &context));
    FT_ASSERT_EQ(-1, pf_run_stress_snprintf_with_limit(6, 'b', "x"));
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('b'));
    return (1);
}

FT_TEST(test_printf_alloc_limit_success_with_high_limit,
    "high alloc limit allows stress custom formatting success")
{
    char chunk[257];
    char format[3];
    char output_buffer[9000];
    t_pf_stress_context context;
    t_pf_snprintf_plain call_pf_snprintf;
    int result;

    pf_fill_repeated(chunk, 256, 'v');
    context.chunk = chunk;
    context.repeat_count = 24;
    format[0] = '%';
    format[1] = 'e';
    format[2] = '\0';
    pf_unregister_custom_specifier('e');
    FT_ASSERT_EQ(0, pf_register_custom_specifier('e', pf_custom_stress_handler,
            &context));
    call_pf_snprintf = pf_snprintf;
    cma_set_alloc_limit(200);
    result = call_pf_snprintf(output_buffer, sizeof(output_buffer), format);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(6144, result);
    FT_ASSERT_EQ('v', output_buffer[0]);
    FT_ASSERT_EQ('v', output_buffer[6143]);
    FT_ASSERT_EQ(0, pf_unregister_custom_specifier('e'));
    return (1);
}
