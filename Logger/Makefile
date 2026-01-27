TARGET := Logger.a
DEBUG_TARGET := Logger_debug.a

SRCS := \
    logger_log_state.cpp \
    logger_log_rotate.cpp \
    logger_log_level_to_str.cpp \
    logger_log_vwrite.cpp \
    logger_log_set_level.cpp \
    logger_log_set_file.cpp \
    logger_log_set_rotation.cpp \
    logger_log_add_sink.cpp \
    logger_log_remove_sink.cpp \
    logger_log_set_alloc_logging.cpp \
    logger_log_get_alloc_logging.cpp \
    logger_log_set_api_logging.cpp \
    logger_log_get_api_logging.cpp \
    logger_log_set_color.cpp \
    logger_log_get_color.cpp \
    logger_log_redaction.cpp \
    logger_log_field_thread_safety.cpp \
    logger_log_sink_thread_safety.cpp \
    logger_file_sink_thread_safety.cpp \
    logger_network_sink_thread_safety.cpp \
    logger_log_close.cpp \
    logger_log_health.cpp \
    logger_lock_contention.cpp \
    logger_log_async_metrics_thread_safety.cpp \
    logger_log_debug.cpp \
    logger_log_info.cpp \
    logger_log_warn.cpp \
    logger_log_error.cpp \
    logger_log_context.cpp \
    logger_log_context_guard.cpp \
    logger_log_structured.cpp \
    logger_log_async.cpp \
    logger.cpp \
    logger_syslog.cpp \
    logger_network.cpp

MODULE_NAME := $(notdir $(CURDIR))
TOTAL_SRCS := $(words $(SRCS))

HEADERS := logger.hpp logger_internal.hpp

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

CFLAGS   ?= -Wall -Wextra -Werror -g -O0 -std=c++17 -Wuseless-cast -pthread

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	@$(CXX) $(CFLAGS) -c $< -o $@
	@count=$$(find $(OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
	    printf '        [%s] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s Debug] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
	@$(CXX) $(CFLAGS) -c $< -o $@
	@count=$$(find $(DEBUG_OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
	    printf '        [%s Debug] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

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
