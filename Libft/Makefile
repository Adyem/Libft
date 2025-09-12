TARGET := LibFT.a
DEBUG_TARGET := LibFT_debug.a

SRCS := libft_atoi.cpp \
    libft_bzero.cpp \
    libft_memchr.cpp \
    libft_memcmp.cpp \
    libft_memcpy.cpp \
    libft_memmove.cpp \
    libft_memdup.cpp \
    libft_memset.cpp \
    libft_strchr.cpp \
    libft_strlcat.cpp \
    libft_strlcpy.cpp \
    libft_strlen.cpp \
    libft_strncmp.cpp \
    libft_strnstr.cpp \
    libft_strstr.cpp \
    libft_strrchr.cpp \
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
    libft_strjoin_multiple.cpp \
    libft_strmapi.cpp \
    libft_striteri.cpp \
    libft_isspace.cpp \
    libft_strlen_size_t.cpp \
    libft_getenv.cpp \
    libft_setenv.cpp \
    libft_unsetenv.cpp \
    libft_fopen.cpp \
    libft_fclose.cpp \
    libft_fgets.cpp \
    libft_fread.cpp \
    libft_fwrite.cpp \
    libft_fseek.cpp \
    libft_ftell.cpp \
    libft_time.cpp

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

CFLAGS   ?= -Wall -Wextra -Werror -g -O0 -std=c++17

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	$(CXX) $(CFLAGS) -c $< -o $@

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(DEBUG_OBJDIR):
	$(MKDIR) $@

CLEAN_OBJS := $(wildcard $(OBJDIR)/*.o) $(wildcard $(DEBUG_OBJDIR)/*.o)

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
else
    CLEAN_FILES := $(CLEAN_OBJS)
endif

clean:
	-$(RM) $(CLEAN_FILES)

fclean: clean
	-$(RM) $(TARGET) $(DEBUG_TARGET)

re: fclean all

both: all debug

.PHONY: all clean fclean re debug both
