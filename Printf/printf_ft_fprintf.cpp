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
        return (FT_ERR_INVALID_ARGUMENT);
    if (data_length == 0)
        return (FT_ERR_SUCCESS);
    if (*written_count > SIZE_MAX - data_length)
    {
        *written_count = SIZE_MAX;
        return (FT_ERR_OUT_OF_RANGE);
    }
    size_t written;
    errno = 0;
    written = fwrite(data_pointer, 1, data_length, writer_context->stream);
    if (written != data_length)
    {
        *written_count = SIZE_MAX;
        if (errno != 0)
            return (FT_ERR_IO);
        return (FT_ERR_IO);
    }
    *written_count += data_length;
    return (FT_ERR_SUCCESS);
}

int ft_vfprintf(FILE *stream, const char *format, va_list args)
{
    int error_code;

    if (stream == ft_nullptr || format == ft_nullptr)
        return (-1);
    pf_stream_writer_context context;
    context.stream = stream;
    size_t written_count;
    written_count = 0;
    va_list current_args;
    va_copy(current_args, args);
    int engine_status;
    engine_status = pf_engine_format(format, current_args, pf_stream_writer, &context, &written_count);
    if (engine_status != FT_ERR_SUCCESS)
    {
        va_end(current_args);
        return (-1);
    }
    va_end(current_args);
    error_code = pf_flush_stream(stream);
    if (error_code != FT_ERR_SUCCESS)
        return (-1);
    if (written_count > static_cast<size_t>(INT_MAX))
        return (-1);
    return (static_cast<int>(written_count));
}

int ft_fprintf(FILE *stream, const char *format, ...)
{
    va_list args;
    int result;

    if (stream == ft_nullptr || format == ft_nullptr)
        return (-1);
    va_start(args, format);
    result = ft_vfprintf(stream, format, args);
    va_end(args);
    return (result);
}
