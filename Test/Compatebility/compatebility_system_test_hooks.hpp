#ifndef COMPATEBILITY_SYSTEM_TEST_HOOKS_HPP
#define COMPATEBILITY_SYSTEM_TEST_HOOKS_HPP

void    cmp_set_force_cpu_count_success(unsigned int cpu_count);
void    cmp_set_force_cpu_count_failure(int errno_value);
void    cmp_clear_force_cpu_count_result(void);
void    cmp_set_force_total_memory_success(unsigned long long memory_size);
void    cmp_set_force_total_memory_failure(int errno_value);
#if defined(_WIN32) || defined(_WIN64)
void    cmp_set_force_total_memory_windows_failure(unsigned long last_error);
#endif
void    cmp_clear_force_total_memory_result(void);

#endif
