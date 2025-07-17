TARGET := libft_tests
DEBUG_TARGET := libft_tests_debug

SRCS := main.cpp atoi_tests.cpp isdigit_tests.cpp memset_tests.cpp strcmp_tests.cpp strlen_tests.cpp toupper_tests.cpp html_tests.cpp networking_tests.cpp extra_libft_tests.cpp cpp_class_tests.cpp template_tests.cpp printf_tests.cpp get_next_line_tests.cpp cma_tests.cpp game_tests.cpp

ifeq ($(OS),Windows_NT)
    MKDIR = mkdir
    RM    = del /F /Q
else
    MKDIR = mkdir -p
    RM    = rm -f
endif

ifdef COMPILE_FLAGS
    CFLAGS := $(COMPILE_FLAGS)
endif

CXX       := g++
CFLAGS   ?= -Wall -Wextra -Werror -g -O0 -std=c++17

OBJDIR       := objs
DEBUG_OBJDIR := objs_debug

OBJS       := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUG_OBJDIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS) ../Full_Libft.a
	$(CXX) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	$(MKDIR) $@

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS) ../Full_Libft_debug.a
	$(CXX) $(CFLAGS) -o $@ $^

$(DEBUG_OBJDIR)/%.o: %.cpp | $(DEBUG_OBJDIR)
	$(CXX) $(CFLAGS) -c $< -o $@

$(DEBUG_OBJDIR):
	$(MKDIR) $@

../Full_Libft.a:
	$(MAKE) -C ..

../Full_Libft_debug.a:
	$(MAKE) -C .. debug

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
