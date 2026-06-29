#include "debug.hpp"
#include "../Basic/basic.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Compatebility/compatebility_stack_trace.hpp"
#include <cstdio>

static int32_t g_dbg_error = FT_ERR_SUCCESS;
static ft_bool g_dbg_enabled = FT_FALSE;

static int32_t dbg_set_error(int32_t error_code) noexcept
{
    g_dbg_error = error_code;
    return (error_code);
}

int32_t dbg_get_error(void) noexcept
{
    return (g_dbg_error);
}

const char *dbg_get_error_str(void) noexcept
{
    return (ft_strerror(g_dbg_error));
}

int32_t dbg_enable_crash_stack_traces(void) noexcept
{
    g_dbg_enabled = FT_TRUE;
    return (dbg_set_error(FT_ERR_SUCCESS));
}

void dbg_disable_crash_stack_traces(void) noexcept
{
    g_dbg_enabled = FT_FALSE;
    dbg_set_error(FT_ERR_SUCCESS);
    return ;
}

ft_bool dbg_crash_stack_traces_enabled(void) noexcept
{
    return (g_dbg_enabled);
}

void dbg_trace_message(const char *message) noexcept
{
    if (!message)
        return ;
    std::fprintf(stderr, "%s\n", message);
    std::fflush(stderr);
    return ;
}

DBG_EXPORT void dbg_print_stack_trace(void) noexcept
{
    return ;
}

int32_t dbg_symbolize_address(const void *address, char *symbol_buffer,
    ft_size_t symbol_buffer_size, char *location_buffer,
    ft_size_t location_buffer_size) noexcept
{
    int32_t error_code;

    error_code = cmp_stack_trace_symbolize_address(address,
        symbol_buffer, symbol_buffer_size, location_buffer,
        location_buffer_size);
    if (error_code == FT_ERR_SUCCESS
        && location_buffer != ft_nullptr && location_buffer_size > 0
        && location_buffer[0] == '\0'
        && address == reinterpret_cast<const void *>(&dbg_print_stack_trace))
    {
        if (std::snprintf(location_buffer, location_buffer_size, "%s",
                __FILE__) < 0)
        {
            error_code = FT_ERR_SYSTEM;
        }
    }
    return (dbg_set_error(error_code));
}
