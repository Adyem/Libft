#ifndef LIBFT_ENVIRONMENT_LOCK_HPP
#define LIBFT_ENVIRONMENT_LOCK_HPP

int ft_environment_lock(void);
int ft_environment_unlock(void);
int ft_environment_enable_thread_safety(void);
void ft_environment_disable_thread_safety(void);

#endif
