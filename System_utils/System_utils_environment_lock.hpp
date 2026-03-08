#ifndef LIBFT_ENVIRONMENT_LOCK_HPP
#define LIBFT_ENVIRONMENT_LOCK_HPP

#include <cstdint>

int32_t ft_environment_lock(void);
int32_t ft_environment_unlock(void);
int32_t ft_environment_enable_thread_safety(void);
void ft_environment_disable_thread_safety(void);

#endif
