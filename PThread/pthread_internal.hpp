#ifndef PTHREAD_INTERNAL_HPP
# define PTHREAD_INTERNAL_HPP

#include <pthread.h>
#include "recursive_mutex.hpp"
#include "mutex.hpp"

int pt_recursive_mutex_lock_with_error(const pt_recursive_mutex &mutex);
int pt_recursive_mutex_unlock_with_error(const pt_recursive_mutex &mutex);
int pt_mutex_lock_with_error(const pt_mutex &mutex);
int pt_mutex_unlock_with_error(const pt_mutex &mutex);
int pt_mutex_lock_if_valid(pt_mutex *mutex);
int pt_mutex_unlock_if_valid(pt_mutex *mutex);
int pt_recursive_mutex_lock_if_valid(pt_recursive_mutex *mutex);
int pt_recursive_mutex_unlock_if_valid(pt_recursive_mutex *mutex);
int pt_recursive_mutex_lock_if_enabled(pt_recursive_mutex *mutex, bool thread_safe_enabled);
int pt_recursive_mutex_unlock_if_enabled(pt_recursive_mutex *mutex, bool thread_safe_enabled);
int pt_pthread_mutex_lock_with_error(pthread_mutex_t *mutex);
int pt_pthread_mutex_unlock_with_error(pthread_mutex_t *mutex);
int pt_pthread_mutex_try_lock_with_error(pthread_mutex_t *mutex);

int pt_mutex_create_with_error(pt_mutex **mutex);
void pt_mutex_destroy(pt_mutex **mutex);
int pt_recursive_mutex_create_with_error(pt_recursive_mutex **mutex);
void pt_recursive_mutex_destroy(pt_recursive_mutex **mutex);

#endif
