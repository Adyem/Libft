TARGET := Threading.a
DEBUG_TARGET := Threading_debug.a

SRCS := threading_thread.cpp \
        threading_compile_cancellation.cpp \
        threading_cancellation.cpp \
        threading_compile_thread_pool.cpp \
        threading_thread_pool.cpp \
        threading_concurrency.cpp \
        threading_task_scheduler.cpp \
        threading_task_scheduler_tracing.cpp

HEADERS := thread.hpp cancellation.hpp thread_pool.hpp concurrency.hpp task_scheduler.hpp task_scheduler_tracing.hpp lock_guard.hpp unique_lock.hpp errno_guard.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
