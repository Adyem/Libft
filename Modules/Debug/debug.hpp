#ifndef DEBUG_HPP
# define DEBUG_HPP

# include "../Errno/errno.hpp"

# define DBG_STACK_TRACE_MAX_FRAMES 64

int32_t dbg_enable_crash_stack_traces(void) noexcept;
void    dbg_disable_crash_stack_traces(void) noexcept;
ft_bool dbg_crash_stack_traces_enabled(void) noexcept;
void    dbg_trace_message(const char *message) noexcept;
void    dbg_print_stack_trace(void) noexcept;
int32_t dbg_symbolize_address(const void *address,
            char *symbol_buffer, ft_size_t symbol_buffer_size,
            char *location_buffer, ft_size_t location_buffer_size) noexcept;
int32_t dbg_get_error(void) noexcept;
const char *dbg_get_error_str(void) noexcept;

#endif
