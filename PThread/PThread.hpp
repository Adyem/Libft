#ifndef PTHREAD_HPP
# define PTHREAD_HPP

#include <pthread.h>

int pt_thread_join(pthread_t thread, void **retval);
int pt_thread_create(pthread_t *thread, const pthread_attr_t *attr,
		void *(*start_routine)(void *), void *arg);

#define SLEEP_TIME 100
#define MAX_SLEEP 10000
#define MAX_QUEUE 128

#endif
