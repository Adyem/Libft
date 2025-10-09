#ifndef LIBFT_ENVIRONMENT_LOCK_HPP
#define LIBFT_ENVIRONMENT_LOCK_HPP

#include "libft_config.hpp"

#if LIBFT_ENABLE_ENVIRONMENT_HELPERS
int ft_environment_lock(void);
int ft_environment_unlock(void);
void ft_environment_force_lock_failure(int error_code);
void ft_environment_force_unlock_failure(int error_code);
void ft_environment_reset_failures(void);
#endif

#endif
