#include "printf.hpp"
#include "printf_internal.hpp"
#include "printf_engine.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdio>
#include <cstdarg>
#include <climits>
#include <cerrno>

struct pf_stream_writer_context
{
    FILE *stream;
};

static int pf_stream_writer(const char *data_pointer, size_t data_length, void *context, size_t *written_count)
{
    pf_stream_writer_context *writer_context;

    writer_context = static_cast<pf_stream_writer_context*>(context);
    if (!writer_context || !writer_context->stream || !written_count)
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
    size_t written;
    errno = 0;
    written = fwrite(data_pointer, 1, data_length, writer_context->stream);
    if (written != data_length)
    {
        if (errno != 0)
            ft_errno = ft_map_system_error(errno);
        else
            ft_errno = FT_ERR_IO;
        *written_count = SIZE_MAX;
        return (-1);
    }
    *written_count += data_length;
    return (0);
}

int ft_vfprintf(FILE *stream, const char *format, va_list args)
{
    if (stream == ft_nullptr || format == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    pf_stream_writer_context context;
    context.stream = stream;
    size_t written_count;
    written_count = 0;
    va_list current_args;
    va_copy(current_args, args);
    int engine_status;
    engine_status = pf_engine_format(format, current_args, pf_stream_writer, &context, &written_count);
    if (engine_status != 0)
    {
        int format_error;

        format_error = ft_errno;
        va_end(current_args);
        ft_errno = format_error;
        if (written_count != SIZE_MAX && ft_errno == FT_ER_SUCCESSS)
            ft_errno = FT_ERR_IO;
        return (-1);
    }
    va_end(current_args);
    if (pf_flush_stream(stream) != 0)
        return (-1);
    if (written_count > static_cast<size_t>(INT_MAX))
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (static_cast<int>(written_count));
}

int ft_fprintf(FILE *stream, const char *format, ...)
{
    va_list args;
    int result;

    if (stream == ft_nullptr || format == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    va_start(args, format);
    result = ft_vfprintf(stream, format, args);
    va_end(args);
    return (result);
}

