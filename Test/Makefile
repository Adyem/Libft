TARGET := libft_tests
DEBUG_TARGET := libft_tests_debug

EFF_SRCS := $(wildcard Efficiency/*.cpp)

SRCS := main.cpp $(wildcard Test/*.cpp)

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
OPT_FLAGS = -O1 \
        -fno-builtin -fno-builtin-memcpy -fno-builtin-memmove -fno-builtin-memset \
        -fno-builtin-strlen -fno-builtin-strcmp -fno-builtin-isdigit
else ifeq ($(OPT_LEVEL),2)
OPT_FLAGS = -O2 \
        -fno-builtin -fno-builtin-memcpy -fno-builtin-memmove -fno-builtin-memset \
        -fno-builtin-strlen -fno-builtin-strcmp -fno-builtin-isdigit
else ifeq ($(OPT_LEVEL),3)
OPT_FLAGS = -O3 \
        -fno-builtin -fno-builtin-memcpy -fno-builtin-memmove -fno-builtin-memset \
        -fno-builtin-strlen -fno-builtin-strcmp -fno-builtin-isdigit
else
$(error Unsupported OPT_LEVEL=$(OPT_LEVEL))
endif

CXX       := g++

COMPILE_FLAGS := -Wall -Wextra -Werror -std=c++17 $(OPT_FLAGS) -pthread -Wno-missing-declarations
CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Libft\"
LDFLAGS := -lz -lssl -lcrypto
export COMPILE_FLAGS

OBJDIR       := objs
DEBUG_OBJDIR := objs_debug

OBJS       := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUG_OBJDIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS) ../Full_Libft.a
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJDIR)/Test/test_promise.o $(OBJDIR)/Test/test_task_scheduler.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Concurrency\"
$(OBJDIR)/Test/test_networking.o $(OBJDIR)/Test/test_http_server.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Networking\"
$(OBJDIR)/Test/test_logger.o $(OBJDIR)/Test/test_logger_async.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Logger\"
$(OBJDIR)/Test/test_json_validate.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"JSon\"
$(OBJDIR)/Test/test_yaml.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"YAML\"
$(OBJDIR)/Test/test_rng.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"RNG\"
$(OBJDIR)/Test/test_linear_algebra.o $(OBJDIR)/Test/test_quaternion.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Math\"
$(OBJDIR)/Test/test_string_view.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"String\"
$(OBJDIR)/Test/test_pathfinding.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_encryption_key.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Encryption\"

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS) ../Full_Libft_debug.a
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(DEBUG_OBJDIR)/%.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@

../Full_Libft.a:
	$(MAKE) -C .. COMPILE_FLAGS="$(COMPILE_FLAGS)"

../Full_Libft_debug.a:
	$(MAKE) -C .. debug COMPILE_FLAGS="$(COMPILE_FLAGS)"

CLEAN_OBJS := $(shell find $(OBJDIR) $(DEBUG_OBJDIR) -type f -name '*.o' 2>/dev/null)

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
else
    CLEAN_FILES := $(CLEAN_OBJS)
endif

clean:
	-$(RM) $(CLEAN_FILES)
	$(MAKE) -C .. clean COMPILE_FLAGS="$(COMPILE_FLAGS)"

fclean:
	-$(RM) $(CLEAN_FILES)
	-$(RM) $(TARGET) $(DEBUG_TARGET)
	$(MAKE) -C .. fclean COMPILE_FLAGS="$(COMPILE_FLAGS)"

re: fclean all

both: all debug

.PHONY: all clean fclean re debug both
