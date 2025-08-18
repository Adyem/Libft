TARGET := libft_tests
DEBUG_TARGET := libft_tests_debug

SRCS := main.cpp atoi_tests.cpp isdigit_tests.cpp memset_tests.cpp strcmp_tests.cpp strlen_tests.cpp \
       toupper_tests.cpp html_tests.cpp networking_tests.cpp extra_libft_tests.cpp \
       cpp_class_tests.cpp template_tests.cpp printf_tests.cpp get_next_line_tests.cpp \
       cma_tests.cpp game_tests.cpp queue_tests.cpp queue_class_tests.cpp \
       promise_tests.cpp efficiency_tests.cpp

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
	$(MAKE) -C .. clean

fclean:
	-$(RM) $(CLEAN_FILES)
	-$(RM) $(TARGET) $(DEBUG_TARGET)
	$(MAKE) -C .. fclean

re: fclean all

both: all debug

.PHONY: all clean fclean re debug both
