TARGET := LibFT.a
DEBUG_TARGET := LibFT_debug.a

SRCS := libft_atoi.cpp \
    libft_bzero.cpp \
    libft_memchr.cpp \
    libft_memcmp.cpp \
    libft_memcpy.cpp \
    libft_memcpy_s.cpp \
    libft_memmove.cpp \
    libft_memmove_s.cpp \
    libft_memset.cpp \
    libft_strchr.cpp \
    libft_strlcat.cpp \
    libft_strlcpy.cpp \
    libft_strnlen.cpp \
    libft_strncmp.cpp \
    libft_strnstr.cpp \
    libft_strstr.cpp \
    libft_strrchr.cpp \
    libft_strtok.cpp \
    libft_atol.cpp \
    libft_strtol.cpp \
    libft_strtoul.cpp \
    libft_isdigit.cpp \
    libft_isalpha.cpp \
    libft_isalnum.cpp \
    libft_isprint.cpp \
    libft_islower.cpp \
    libft_isupper.cpp \
    libft_strcmp.cpp \
    libft_toupper.cpp \
    libft_tolower.cpp \
    libft_strncpy.cpp \
    libft_strcpy_s.cpp \
    libft_strncpy_s.cpp \
    libft_strcat_s.cpp \
    libft_strncat_s.cpp \
    libft_span.cpp \
    libft_locale.cpp \
    libft_wide.cpp \
    libft_strmapi.cpp \
    libft_striteri.cpp \
    libft_isspace.cpp \
    libft_getenv.cpp \
    libft_environment_lock.cpp \
    libft_setenv.cpp \
    libft_unsetenv.cpp \
    libft_fopen.cpp \
    libft_fclose.cpp \
    libft_fgets.cpp \
    libft_time.cpp \
    libft_validate_int.cpp \
    libft_to_string.cpp \
    libft_utf8_case.cpp \
    libft_utf8_decode.cpp \
    libft_utf8_grapheme.cpp \
    libft_utf8_length.cpp

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

