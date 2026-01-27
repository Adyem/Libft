TARGET := System_utils.a
DEBUG_TARGET := System_utils_debug.a

SRCS := System_utils_assert.cpp \
        System_utils_env.cpp \
        System_utils_abort.cpp \
        System_utils_sigabrt.cpp \
        System_utils_sigfpe.cpp \
        System_utils_sigill.cpp \
        System_utils_sigint.cpp \
        System_utils_sigsegv.cpp \
        System_utils_sigterm.cpp \
        System_utils_sysinfo.cpp \
        System_utils_file_open.cpp \
        System_utils_file_io.cpp \
        System_utils_file_stream.cpp \
        System_utils_file_utils.cpp \
        System_utils_health.cpp \
        System_utils_resource_tracer.cpp \
        System_utils_service.cpp \
        System_utils_home.cpp \
        System_utils_locale.cpp \
        test_runner.cpp

MODULE_NAME := $(notdir $(CURDIR))
TOTAL_SRCS := $(words $(SRCS))

HEADERS := system_utils.hpp test_runner.hpp

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
