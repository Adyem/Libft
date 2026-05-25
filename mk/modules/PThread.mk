TARGET := PThread.a
DEBUG_TARGET := PThread_debug.a

SRCS := pthread_lock_mutex.cpp \
        pthread_unlock_mutex.cpp \
        pthread_try_lock_mutex.cpp \
        pthread_thread_join.cpp \
        pthread_thread_create.cpp \
        pthread_thread_detach.cpp \
        pthread_thread_cancel.cpp \
        pthread_thread_sleep.cpp \
        pthread_thread_yield.cpp \
        pthread_thread_wait.cpp \
        pthread_this_thread.cpp \
        pthread_thread_self.cpp \
        pthread_thread_equal.cpp \
        pthread_thread.cpp \
        pthread_mutex.cpp \
        pthread_recursive_mutex.cpp \
        pthread_recursive_lock_mutex.cpp \
        pthread_recursive_try_lock_mutex.cpp \
        pthread_recursive_unlock_mutex.cpp \
        pthread_atomic_load.cpp \
        pthread_atomic_store.cpp \
        pthread_atomic_fetch_add.cpp \
        pthread_atomic_compare_exchange.cpp \
        pthread_condition.cpp \
        pthread_condition_variable.cpp \
        pthread_rwlock.cpp \
        pthread_concurrency.cpp \
        pthread_task_scheduler.cpp \
        pthread_lock_tracking.cpp \
        pthread_task_scheduler_tracing.cpp

HEADERS := pthread.hpp mutex.hpp condition.hpp thread.hpp task_scheduler.hpp unique_lock.hpp pthread_lock_tracking.hpp recursive_mutex.hpp concurrency.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
