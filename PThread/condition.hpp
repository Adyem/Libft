#ifndef CONDITION_HPP
# define CONDITION_HPP

#include <pthread.h>

int pt_cond_init(pthread_cond_t *condition, const pthread_condattr_t *attributes);
int pt_cond_destroy(pthread_cond_t *condition);
int pt_cond_wait(pthread_cond_t *condition, pthread_mutex_t *mutex);
int pt_cond_signal(pthread_cond_t *condition);
int pt_cond_broadcast(pthread_cond_t *condition);

#endif
