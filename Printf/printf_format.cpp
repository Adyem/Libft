#include "printf_internal.hpp"
#include "printf_engine.hpp"
#include "../Errno/errno.hpp"
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
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (data_length == 0)
        return (0);
    if (*written_count > SIZE_MAX - data_length)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        *written_count = SIZE_MAX;
        return (-1);
    }
    ssize_t written_result;
    written_result = su_write(writer_context->file_descriptor, data_pointer, data_length);
    if (written_result != static_cast<ssize_t>(data_length))
    {
        ft_errno = FT_ERR_IO;
        *written_count = SIZE_MAX;
        return (-1);
    }
    *written_count += data_length;
    return (0);
}

int pf_printf_fd_v(int fd, const char *format, va_list args)
{
    if (fd < 0 || format == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    pf_fd_writer_context context;
    context.file_descriptor = fd;
    size_t written_count;
    written_count = 0;
    va_list current_args;
    va_copy(current_args, args);
    if (pf_engine_format(format, current_args, pf_fd_writer, &context, &written_count) != 0)
    {
        va_end(current_args);
        if (ft_errno == FT_ERR_SUCCESSS)
            ft_errno = FT_ERR_IO;
        return (-1);
    }
    va_end(current_args);
    if (written_count > static_cast<size_t>(INT_MAX))
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (static_cast<int>(written_count));
}

