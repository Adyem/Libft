#include "printf_internal.hpp"
#include "printf_engine.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstdarg>
#include <climits>

struct pf_fd_writer_context
{
    int file_descriptor;
};

static int pf_fd_writer(const char *data_pointer, size_t data_length, void *context, size_t *written_count)
{
    pf_fd_writer_context *writer_context;

    writer_context = static_cast<pf_fd_writer_context*>(context);
    if (!writer_context || !written_count)
        return (FT_ERR_INVALID_ARGUMENT);
    if (data_length == 0)
        return (FT_ERR_SUCCESS);
    if (*written_count > SIZE_MAX - data_length)
    {
        *written_count = SIZE_MAX;
        return (FT_ERR_OUT_OF_RANGE);
    }
    ssize_t written_result;
    written_result = su_write(writer_context->file_descriptor, data_pointer, data_length);
    if (written_result != static_cast<ssize_t>(data_length))
    {
        *written_count = SIZE_MAX;
        return (FT_ERR_IO);
    }
    *written_count += data_length;
    return (FT_ERR_SUCCESS);
}

int pf_printf_fd_v(int file_descriptor, const char *format, va_list argument_list)
{
    int error_code;

    if (file_descriptor < 0 || format == ft_nullptr)
        return (-1);
    pf_fd_writer_context context;
    context.file_descriptor = file_descriptor;
    size_t written_count;
    written_count = 0;
    va_list current_args;
    va_copy(current_args, argument_list);
    error_code = pf_engine_format(format, current_args, pf_fd_writer, &context, &written_count);
    if (error_code != FT_ERR_SUCCESS)
    {
        va_end(current_args);
        return (-1);
    }
    va_end(current_args);
    if (written_count > static_cast<size_t>(INT_MAX))
        return (-1);
    return (static_cast<int>(written_count));
}
