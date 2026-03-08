#ifndef ERRNO_INTERNAL_HPP
# define ERRNO_INTERNAL_HPP

#include <cstdint>

void    errno_abort_if_uninitialised(uint8_t initialised_state,
            const char *method_name);
void    errno_abort_lifecycle(uint8_t initialised_state, const char *method_name,
            const char *reason);

#endif
