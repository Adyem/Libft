#ifndef COMPATEBILITY_STACK_TRACE_HPP
# define COMPATEBILITY_STACK_TRACE_HPP


#ifndef LIBFT_INTERNAL_HEADERS
# error "This is a libft internal header. Define LIBFT_INTERNAL_HEADERS only when building libft internals."
#endif
# include <cstdio>
# include "../Basic/basic.hpp"
# include "../Errno/errno.hpp"

# define CMP_STACK_TRACE_MAX_FRAMES 16

ft_size_t    cmp_stack_trace_capture(void **frames, ft_size_t capacity,
                ft_size_t skip_count);
int32_t    cmp_stack_trace_symbolize_address(const void *address,
                char *symbol_buffer, ft_size_t symbol_buffer_size,
                char *location_buffer, ft_size_t location_buffer_size);
void    cmp_stack_trace_print(FILE *output_file, void *const *frames,
            ft_size_t frame_count);

#endif
