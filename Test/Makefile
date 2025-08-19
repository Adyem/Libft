TARGET := libft_tests
DEBUG_TARGET := libft_tests_debug

EFF_SRCS := efficiency/efficiency_strlen.cpp efficiency/efficiency_memcpy.cpp \
            efficiency/efficiency_memmove.cpp efficiency/efficiency_memset.cpp \
            efficiency/efficiency_strcmp.cpp efficiency/efficiency_isdigit.cpp \
            efficiency/efficiency_bzero.cpp efficiency/efficiency_memcmp.cpp \
            efficiency/efficiency_strchr.cpp efficiency/efficiency_strncmp.cpp \
            efficiency/efficiency_isalpha.cpp efficiency/efficiency_isalnum.cpp \
            efficiency/efficiency_memchr.cpp efficiency/efficiency_strrchr.cpp \
            efficiency/efficiency_isspace.cpp efficiency/efficiency_abs.cpp \
            efficiency/efficiency_cma_malloc.cpp efficiency/efficiency_cma_calloc.cpp \
            efficiency/efficiency_cma_strdup.cpp efficiency/efficiency_cma_memdup.cpp \
            efficiency/efficiency_cma_realloc.cpp

SRCS := main.cpp test_atoi.cpp test_isdigit.cpp test_memset.cpp test_strcmp.cpp test_strlen.cpp \
       test_toupper.cpp test_html.cpp test_networking.cpp test_extra_libft.cpp \
       test_cpp_class.cpp test_template.cpp test_printf.cpp test_get_next_line.cpp \
       test_cma.cpp test_game.cpp test_queue.cpp test_queue_class.cpp \
       test_promise.cpp $(EFF_SRCS)

ifeq ($(OS),Windows_NT)
    MKDIR = mkdir
    RM    = del /F /Q
else
    MKDIR = mkdir -p
    RM    = rm -f
endif

OPT_LEVEL ?= 3

ifeq ($(OPT_LEVEL),0)
OPT_FLAGS = -O0 -g
else ifeq ($(OPT_LEVEL),1)
OPT_FLAGS = -O1 -flto -s -ffunction-sections -fdata-sections -Wl,--gc-sections
else ifeq ($(OPT_LEVEL),2)
OPT_FLAGS = -O2 -flto -s -ffunction-sections -fdata-sections -Wl,--gc-sections
else ifeq ($(OPT_LEVEL),3)
OPT_FLAGS = -O3 -flto -s -ffunction-sections -fdata-sections -Wl,--gc-sections
else
$(error Unsupported OPT_LEVEL=$(OPT_LEVEL))
endif

CXX       := g++

ifdef COMPILE_FLAGS
CFLAGS := $(COMPILE_FLAGS) $(OPT_FLAGS)
else
CFLAGS ?= -Wall -Wextra -Werror -std=c++17 $(OPT_FLAGS)
endif

OBJDIR       := objs
DEBUG_OBJDIR := objs_debug

OBJS       := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUG_OBJDIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS) ../Full_Libft.a
	$(CXX) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS) ../Full_Libft_debug.a
	$(CXX) $(CFLAGS) -o $@ $^

$(DEBUG_OBJDIR)/%.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@

../Full_Libft.a:
	$(MAKE) -C ..

../Full_Libft_debug.a:
	$(MAKE) -C .. debug

CLEAN_OBJS := $(shell find $(OBJDIR) $(DEBUG_OBJDIR) -type f -name '*.o' 2>/dev/null)

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
else
    CLEAN_FILES := $(CLEAN_OBJS)
endif

clean:
	-$(RM) $(CLEAN_FILES)
	$(MAKE) -C .. clean

fclean:
	-$(RM) $(CLEAN_FILES)
	-$(RM) $(TARGET) $(DEBUG_TARGET)
	$(MAKE) -C .. fclean

re: fclean all

both: all debug

.PHONY: all clean fclean re debug both
