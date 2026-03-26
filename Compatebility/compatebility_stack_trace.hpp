#ifndef COMPATEBILITY_STACK_TRACE_HPP
# define COMPATEBILITY_STACK_TRACE_HPP

# include <cstdio>
# include "../Basic/basic.hpp"

# define CMP_STACK_TRACE_MAX_FRAMES 16

ft_size_t    cmp_stack_trace_capture(void **frames, ft_size_t capacity,
                ft_size_t skip_count);
void    cmp_stack_trace_print(FILE *output_file, void *const *frames,
            ft_size_t frame_count);

#endif
