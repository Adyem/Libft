#include "printf.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "printf_internal.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

typedef FILE *(*t_pf_tmpfile_function)(void);
typedef int32_t (*t_pf_fflush_function)(FILE *);
typedef int64_t (*t_pf_ftell_function)(FILE *);

static int64_t pf_default_ftell(FILE *stream)
{
    return (ftell(stream));
}

static t_pf_tmpfile_function g_pf_tmpfile_function = tmpfile;
static t_pf_fflush_function g_pf_fflush_function = fflush;
static t_pf_ftell_function g_pf_ftell_function = pf_default_ftell;

void pf_set_tmpfile_function(t_pf_tmpfile_function function)
{
    if (function == ft_nullptr)
        g_pf_tmpfile_function = tmpfile;
    else
        g_pf_tmpfile_function = function;
    return ;
}

void pf_reset_tmpfile_function(void)
{
    g_pf_tmpfile_function = tmpfile;
    return ;
}

void pf_set_fflush_function(t_pf_fflush_function function)
{
    if (function == ft_nullptr)
        g_pf_fflush_function = fflush;
    else
        g_pf_fflush_function = function;
    return ;
}

void pf_reset_fflush_function(void)
{
    g_pf_fflush_function = fflush;
    return ;
}

int32_t pf_flush_stream(FILE *stream)
{
    int32_t flush_status;
    int32_t saved_errno;

    if (stream == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    errno = 0;
    flush_status = g_pf_fflush_function(stream);
    if (flush_status != 0)
    {
        saved_errno = errno;
        if (saved_errno != 0)
            return (FT_ERR_IO);
        return (FT_ERR_IO);
    }
    return (FT_ERR_SUCCESS);
}

void pf_set_ftell_function(t_pf_ftell_function function)
{
    if (function == ft_nullptr)
        g_pf_ftell_function = pf_default_ftell;
    else
        g_pf_ftell_function = function;
    return ;
}

void pf_reset_ftell_function(void)
{
    g_pf_ftell_function = pf_default_ftell;
    return ;
}

int32_t pf_vsnprintf(char *string, ft_size_t size, const char *format, va_list argument_list)
{
    int32_t error_code;

    if (format == ft_nullptr || (string == ft_nullptr && size > 0))
        return (-1);
    FILE *stream = g_pf_tmpfile_function();
    if (stream == ft_nullptr)
    {
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        return (-1);
    }
    va_list copy;
    va_copy(copy, argument_list);
    int32_t printed = ft_vfprintf(stream, format, copy);
    va_end(copy);
    if (printed < 0)
    {
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        fclose(stream);
        return (printed);
    }
    error_code = pf_flush_stream(stream);
    if (error_code != FT_ERR_SUCCESS)
    {
        fclose(stream);
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        return (-1);
    }
    int64_t position = g_pf_ftell_function(stream);
    if (position < 0)
    {
        fclose(stream);
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        return (-1);
    }
    rewind(stream);
    if (string != ft_nullptr && size > 0)
    {
        ft_size_t copy_length = static_cast<ft_size_t>(position);
        if (copy_length >= size)
            copy_length = size - 1;
        ft_size_t read_bytes = 0;
        if (copy_length > 0)
            read_bytes = fread(string, 1, copy_length, stream);
        string[read_bytes] = '\0';
    }
    fclose(stream);
    return (printed);
}
