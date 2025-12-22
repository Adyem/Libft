TARGET := PThread.a
DEBUG_TARGET := PThread_debug.a

SRCS := pthread_lock_mutex.cpp \
        pthread_unlock_mutex.cpp \
        pthread_try_lock_mutex.cpp \
        pthread_timed_lock_mutex.cpp \
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
        pthread_recursive_timed_lock_mutex.cpp \
        pthread_recursive_unlock_mutex.cpp \
        pthread_atomic_load.cpp \
        pthread_atomic_store.cpp \
        pthread_atomic_fetch_add.cpp \
        pthread_atomic_compare_exchange.cpp \
        pthread_condition.cpp \
        pthread_condition_variable.cpp \
        pthread_rwlock.cpp \
        pthread_task_scheduler.cpp \
        pthread_lock_tracking.cpp \
        pthread_task_scheduler_tracing.cpp

MODULE_NAME := $(notdir $(CURDIR))
TOTAL_SRCS := $(words $(SRCS))

HEADERS := pthread.hpp mutex.hpp condition.hpp thread.hpp task_scheduler.hpp unique_lock.hpp pthread_lock_tracking.hpp recursive_mutex.hpp

ifeq ($(OS),Windows_NT)
    MKDIR   = mkdir
    RM      = del /F /Q
else
    MKDIR   = mkdir -p
    RM      = rm -f
endif

ifdef COMPILE_FLAGS
    CFLAGS := $(COMPILE_FLAGS)
endif

CXX       := g++
AR        := ar
ARFLAGS   := rcs

OBJDIR         := objs
DEBUG_OBJDIR   := objs_debug

OBJS       := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUG_OBJDIR)/%.o,$(SRCS))

CFLAGS   ?= -Wall -Wextra -Werror -g -O0 -std=c++17 -Wuseless-cast

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	@$(CXX) $(CFLAGS) -c $< -o $@
	@count=$$(find $(OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
	    printf '        [%s] Files built: %d/%d\n' "$(MODULE_NAME)" $$count $(TOTAL_SRCS)

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s Debug] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
	@$(CXX) $(CFLAGS) -c $< -o $@
	@count=$$(find $(DEBUG_OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
	    printf '        [%s Debug] Files built: %d/%d\n' "$(MODULE_NAME)" $$count $(TOTAL_SRCS)

$(OBJDIR) $(DEBUG_OBJDIR):
	@$(MKDIR) $@

CLEAN_OBJS := $(wildcard $(OBJDIR)/*.o) $(wildcard $(DEBUG_OBJDIR)/*.o)

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
else
    CLEAN_FILES := $(CLEAN_OBJS)
endif

clean:
	@if $(RM) $(CLEAN_FILES); then \
		printf '        [%s] clean: success\n' "$(MODULE_NAME)"; \
	else \
		printf '        [%s] clean: failed\n' "$(MODULE_NAME)"; \
		exit 1; \
	fi

fclean:
	@status=0; \
	if ! $(RM) $(CLEAN_FILES); then \
		status=1; \
	fi; \
	if ! $(RM) $(TARGET) $(DEBUG_TARGET); then \
		status=1; \
	fi; \
	if [ $$status -eq 0 ]; then \
		printf '        [%s] fclean: success\n' "$(MODULE_NAME)"; \
	else \
		printf '        [%s] fclean: failed\n' "$(MODULE_NAME)"; \
		exit 1; \
	fi

re: fclean all

both: all debug

.PHONY: all clean fclean re debug both
