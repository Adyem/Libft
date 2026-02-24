include ../compiler_flags.mk
TARGET := Basic.a
DEBUG_TARGET := Basic_debug.a

SRCS := basic_atoi.cpp \
    basic_bzero.cpp \
    basic_memchr.cpp \
    basic_memcmp.cpp \
    basic_memcpy.cpp \
    basic_memcpy_s.cpp \
    basic_memmove.cpp \
    basic_memmove_s.cpp \
    basic_memset.cpp \
    basic_strchr.cpp \
    basic_strlcat.cpp \
    basic_strlcpy.cpp \
    basic_strnlen.cpp \
    basic_strncmp.cpp \
    basic_strnstr.cpp \
    basic_strstr.cpp \
    basic_strrchr.cpp \
    basic_strtok.cpp \
    basic_atol.cpp \
    basic_strtol.cpp \
    basic_strtoul.cpp \
    basic_isdigit.cpp \
    basic_isalpha.cpp \
    basic_isalnum.cpp \
    basic_isprint.cpp \
    basic_islower.cpp \
    basic_isupper.cpp \
    basic_strcmp.cpp \
    basic_toupper.cpp \
    basic_tolower.cpp \
    basic_strncpy.cpp \
    basic_strcpy_s.cpp \
    basic_strncpy_s.cpp \
    basic_strcat_s.cpp \
    basic_strncat_s.cpp \
    basic_span_dup.cpp \
    basic_locale_compare.cpp \
    basic_wstrlen.cpp \
    basic_strmapi.cpp \
    basic_striteri.cpp \
    basic_isspace.cpp \
    basic_validate_int.cpp \
    basic_utf8_case.cpp \
    basic_utf8_decode.cpp \
    basic_utf8_grapheme.cpp \
    basic_utf8_length.cpp

MODULE_NAME := $(notdir $(CURDIR))
TOTAL_SRCS := $(words $(SRCS))

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

BUILD_OBJ_SUFFIX :=
ifneq ($(findstring -DLIBFT_TEST_BUILD,$(COMPILE_FLAGS)),)
    BUILD_OBJ_SUFFIX := _test
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
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@
	@count=$$(find $(OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
	    printf '        [%s] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s Debug] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@
	@count=$$(find $(DEBUG_OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
	    printf '        [%s Debug] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

$(OBJDIR) $(DEBUG_OBJDIR):
	@$(MKDIR) $@

CLEAN_OBJS := $(wildcard $(OBJDIR)/*.o) $(wildcard $(OBJDIR)/*.d) $(wildcard $(DEBUG_OBJDIR)/*.o) $(wildcard $(DEBUG_OBJDIR)/*.d)

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

-include $(DEPS) $(DEBUG_DEPS)

.PHONY: all clean fclean re debug both
