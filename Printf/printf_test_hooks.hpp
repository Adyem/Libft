#ifndef PRINTF_TEST_HOOKS_HPP
# define PRINTF_TEST_HOOKS_HPP

#include <stddef.h>

typedef size_t (*t_pf_printf_count_override_hook)(void);

void    pf_printf_set_count_override_hook(t_pf_printf_count_override_hook hook);

#endif
