#ifndef ERRNO_INTERNAL_HPP
# define ERRNO_INTERNAL_HPP


#ifndef LIBFT_INTERNAL_HEADERS
# error "This is a libft internal header. Define LIBFT_INTERNAL_HEADERS only when building libft internals."
#endif
#include <cstdint>

void    errno_abort_if_uninitialised_or_destroyed(uint8_t initialised_state,
            const char *method_name);
void    errno_abort_if_uninitialised(uint8_t initialised_state,
            const char *method_name);
void    errno_abort_lifecycle(uint8_t initialised_state, const char *method_name,
            const char *reason);

#endif
