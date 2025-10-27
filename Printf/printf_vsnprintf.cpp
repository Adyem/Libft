#include "printf.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "printf_internal.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

typedef FILE *(*t_pf_tmpfile_function)(void);
typedef int (*t_pf_fflush_function)(FILE *);
typedef long (*t_pf_ftell_function)(FILE *);

static t_pf_tmpfile_function g_pf_tmpfile_function = tmpfile;
static t_pf_fflush_function g_pf_fflush_function = fflush;
static t_pf_ftell_function g_pf_ftell_function = ftell;

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

int pf_flush_stream(FILE *stream)
{
    int flush_status;
    int saved_errno;

    if (stream == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    errno = 0;
    flush_status = g_pf_fflush_function(stream);
    if (flush_status != 0)
    {
        saved_errno = errno;
        if (saved_errno != 0)
            ft_errno = ft_map_system_error(saved_errno);
        else
            ft_errno = FT_ERR_IO;
        return (-1);
    }
    return (0);
}

void pf_set_ftell_function(t_pf_ftell_function function)
{
    if (function == ft_nullptr)
        g_pf_ftell_function = ftell;
    else
        g_pf_ftell_function = function;
    return ;
}

void pf_reset_ftell_function(void)
{
    g_pf_ftell_function = ftell;
    return ;
}

int pf_vsnprintf(char *string, size_t size, const char *format, va_list args)
{
    if (format == ft_nullptr || (string == ft_nullptr && size > 0))
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    FILE *stream = g_pf_tmpfile_function();
    if (stream == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        return (-1);
    }
    va_list copy;
    va_copy(copy, args);
    int printed = ft_vfprintf(stream, format, copy);
    va_end(copy);
    if (printed < 0)
    {
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        fclose(stream);
        return (printed);
    }
    if (pf_flush_stream(stream) != 0)
    {
        fclose(stream);
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        return (-1);
    }
    long position = g_pf_ftell_function(stream);
    if (position < 0)
    {
        int saved_errno = errno;
        ft_errno = ft_map_system_error(saved_errno);
        fclose(stream);
        if (string != ft_nullptr && size > 0)
            string[0] = '\0';
        return (-1);
    }
    rewind(stream);
    if (string != ft_nullptr && size > 0)
    {
        size_t copy_length = static_cast<size_t>(position);
        if (copy_length >= size)
            copy_length = size - 1;
        size_t read_bytes = 0;
        if (copy_length > 0)
            read_bytes = fread(string, 1, copy_length, stream);
        string[read_bytes] = '\0';
    }
    fclose(stream);
    ft_errno = ER_SUCCESS;
    return (printed);
}

