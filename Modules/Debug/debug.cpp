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

void dbg_print_stack_trace(void) noexcept
{
    void *frames[DBG_STACK_TRACE_MAX_FRAMES];
    ft_size_t frame_count;

    frame_count = cmp_stack_trace_capture(frames,
            DBG_STACK_TRACE_MAX_FRAMES, 1);
    if (frame_count == 0)
        return ;
    std::fprintf(stderr, "libft debug: stack trace:\n");
    cmp_stack_trace_print(stderr, frames, frame_count);
    std::fflush(stderr);
    return ;
}

int32_t dbg_symbolize_address(const void *address, char *symbol_buffer,
    ft_size_t symbol_buffer_size, char *location_buffer,
    ft_size_t location_buffer_size) noexcept
{
    return (dbg_set_error(cmp_stack_trace_symbolize_address(address,
        symbol_buffer, symbol_buffer_size, location_buffer,
        location_buffer_size)));
}
