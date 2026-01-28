#include "readline_internal.hpp"
#include "../Errno/errno.hpp"

static thread_local int g_readline_internal_error = FT_ERR_SUCCESSS;

void rl_internal_set_error(int error_code)
{
    g_readline_internal_error = error_code;
    return ;
}

int rl_internal_get_error(void)
{
    return (g_readline_internal_error);
}

int rl_internal_consume_error(void)
{
    int error_code;

    error_code = g_readline_internal_error;
    g_readline_internal_error = FT_ERR_SUCCESSS;
    return (error_code);
}
