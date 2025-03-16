TARGET := LibFT.a
DEBUG_TARGET := LibFT_debug.a

SRCS := ft_atoi.cpp \
	ft_bzero.cpp \
	ft_memchr.cpp \
	ft_memcmp.cpp \
	ft_memcpy.cpp \
	ft_memmove.cpp \
	ft_memset.cpp \
	ft_strchr.cpp \
	ft_strlcat.cpp \
	ft_strlcpy.cpp \
	ft_strlen.cpp \
	ft_strncmp.cpp \
	ft_strnstr.cpp \
	ft_strrchr.cpp \
	ft_atol.cpp \
	ft_isdigit.cpp \
	ft_strcmp.cpp \
	ft_opendir.cpp \
	ft_toupper.cpp \
	ft_tolower.cpp

ifeq ($(OS),Windows_NT)
    MKDIR   = mkdir
    RMDIR   = rmdir /S /Q
    RM      = del /F /Q
else
    MKDIR   = mkdir -p
    RMDIR   = rm -rf
    RM      = rm -f
endif

ifeq ($(ENABLE_LTO),1)
    CFLAGS += -flto
    LDFLAGS += -flto
endif

ifeq ($(ENABLE_PGO),1)
    CFLAGS += -fprofile-generate
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

CXXFLAGS   := -Wall -Wextra -Werror -g -O0 -std=c++17

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR) $(DEBUG_OBJDIR):
	$(MKDIR) $@

clean:
	-$(RMDIR) $(OBJDIR)
	-$(RMDIR) $(DEBUG_OBJDIR)

fclean: clean
	-$(RM) $(TARGET) $(DEBUG_TARGET)

re: fclean all

both: all debug

.PHONY: all clean fclean re debug both
