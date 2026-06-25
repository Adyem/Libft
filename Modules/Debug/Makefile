include ../../mk/compiler_flags.mk
TARGET := Debug.a
DEBUG_TARGET := Debug_debug.a

SRCS := debug.cpp

HEADERS := debug.hpp

MODULE_NAME := $(notdir $(CURDIR))
TOTAL_SRCS := $(words $(SRCS))

ifeq ($(OS),Windows_NT)
    SHELL := C:/Progra~1/Git/bin/bash.exe
    .SHELLFLAGS := -lc
    export SHELL
    export LIBFT_POSIX_SHELL := 1
    ifneq ($(LIBFT_POSIX_SHELL),)
        MKDIR   = mkdir -p
        RM      = rm -f
        RMDIR   = rm -rf
    else
        MKDIR   = mkdir
        RM      = del /F /Q
        RMDIR   = rmdir /S /Q
    endif
else
    MKDIR   = mkdir -p
    RM      = rm -f
    RMDIR   = rm -rf
endif

ifdef COMPILE_FLAGS
    CFLAGS := $(COMPILE_FLAGS)
endif

CXX       := g++
AR        := ar
ARFLAGS   := rcs

BUILD_OBJ_SUFFIX := $(BUILD_OUTPUT_SUFFIX)
ifneq ($(findstring -DLIBFT_TEST_BUILD,$(COMPILE_FLAGS)),)
    BUILD_OBJ_SUFFIX := _test$(BUILD_OUTPUT_SUFFIX)
endif

OBJDIR         := objs$(BUILD_OBJ_SUFFIX)
DEBUG_OBJDIR   := objs_debug$(BUILD_OBJ_SUFFIX)

OBJS       := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUG_OBJDIR)/%.o,$(SRCS))
DEPS       := $(OBJS:.o=.d)
DEBUG_DEPS := $(DEBUG_OBJS:.o=.d)

CFLAGS   ?= -Wall -Wextra -Werror -g -O0 -std=c++17 -Wuseless-cast

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(RM) $@
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@
	$(FIXDEP)
	@count=$$(find $(OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
		printf '        [%s] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	@$(RM) $@
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s Debug] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@
	$(FIXDEP)
	@count=$$(find $(DEBUG_OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
		printf '        [%s Debug] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

$(OBJDIR) $(DEBUG_OBJDIR):
	@$(MKDIR) $@

CLEAN_OBJS := $(shell find . -type f \( -name *.o -o -name *.d -o -name *.gcda -o -name *.gcno \) 2>/dev/null)
CLEAN_DIRS := $(wildcard objs*)
TEST_ARCHIVES := $(wildcard *_test.a) $(wildcard *_debug_test.a)
GENERATED_FILES := compile_commands.json

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\,$(CLEAN_OBJS))
    CLEAN_DIRS := $(subst /,\,$(CLEAN_DIRS))
    FCLEAN_FILES := $(CLEAN_FILES) $(TARGET) $(DEBUG_TARGET) $(TEST_ARCHIVES) $(GENERATED_FILES)
else
    CLEAN_FILES := $(CLEAN_OBJS)
    FCLEAN_FILES := $(CLEAN_FILES) $(TARGET) $(DEBUG_TARGET) $(TEST_ARCHIVES) $(GENERATED_FILES)
endif

clean:
	@status=0; \
	if ! $(RM) $(CLEAN_FILES); then \
		status=1; \
	fi; \
	if ! $(RMDIR) $(CLEAN_DIRS); then \
		status=1; \
	fi; \
	if [ $$status -eq 0 ]; then \
		printf '        [%s] clean: success\n' "$(MODULE_NAME)"; \
	else \
		printf '        [%s] clean: failed\n' "$(MODULE_NAME)"; \
		exit 1; \
	fi

fclean:
	@status=0; \
	if ! $(RM) $(FCLEAN_FILES); then \
		status=1; \
	fi; \
	if ! $(RMDIR) $(CLEAN_DIRS); then \
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


%_test.a: FORCE
	@$(MAKE) TARGET="$@" $(SUBMAKE_OVERRIDES) COMPILE_FLAGS="$(COMPILE_FLAGS) -DLIBFT_TEST_BUILD" all

-include $(DEPS) $(DEBUG_DEPS)

.PHONY: all clean fclean re debug both
