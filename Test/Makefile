TARGET := libft_tests
DEBUG_TARGET := libft_tests_debug
LIBFT_ARCHIVE := Full_Libft.a
LIBFT_DEBUG_ARCHIVE := Full_Libft_debug.a

EFF_SRCS := $(wildcard Efficiency/*.cpp)

SRCS := main.cpp $(wildcard Test/*.cpp) $(wildcard API/*.cpp)

MODULE_NAME := $(notdir $(CURDIR))
TOTAL_SRCS := $(words $(SRCS))

MAKEFLAGS += --no-print-directory

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
LDFLAGS := -lz -lssl -lcrypto -ldl
export COMPILE_FLAGS

OBJDIR       := objs
DEBUG_OBJDIR := objs_debug

OBJS       := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUG_OBJDIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS) ../Full_Libft.a
	@$(CXX) $(CFLAGS) -o $@ $(OBJS) ../Full_Libft.a $(LDFLAGS)
	@printf '        [%s] Test binary ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.cpp
	@$(MKDIR) $(dir $@)
	@$(CXX) $(CFLAGS) -c $< -o $@
	@count=$$(find $(OBJDIR) -type f -name '*.o' 2>/dev/null | wc -l); \
	    printf '        [%s] Files built: %d/%d\n' "$(MODULE_NAME)" $$count $(TOTAL_SRCS)

$(OBJDIR)/Test/test_promise.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_task_scheduler.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"PThread\"
$(OBJDIR)/Test/test_pthread_thread.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"PThread\"
$(OBJDIR)/Test/test_networking.o $(OBJDIR)/Test/test_http_server.o $(OBJDIR)/Test/test_websocket.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Networking\"
$(OBJDIR)/Test/test_logger.o $(OBJDIR)/Test/test_logger_async.o $(OBJDIR)/Test/test_logger_network.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Logger\"
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
	@$(CXX) $(CFLAGS) -o $@ $(DEBUG_OBJS) ../Full_Libft_debug.a $(LDFLAGS)
	@printf '        [%s Debug] Test binary ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(DEBUG_OBJDIR)/%.o: %.cpp
	@$(MKDIR) $(dir $@)
	@$(CXX) $(CFLAGS) -c $< -o $@
	@count=$$(find $(DEBUG_OBJDIR) -type f -name '*.o' 2>/dev/null | wc -l); \
	    printf '        [%s Debug] Files built: %d/%d\n' "$(MODULE_NAME)" $$count $(TOTAL_SRCS)

../Full_Libft.a:
	@need_build=0; \
	if $(MAKE) -C .. -q $(LIBFT_ARCHIVE) COMPILE_FLAGS="$(COMPILE_FLAGS)"; then \
		printf '        [%s] Full_Libft.a is up to date\n' "$(MODULE_NAME)"; \
	else \
		status=$$?; \
		if [ $$status -eq 1 ]; then \
			need_build=1; \
		else \
			exit $$status; \
		fi; \
	fi; \
	if [ $$need_build -eq 1 ] || [ ! -f $@ ]; then \
		printf '        [%s] Updating Full_Libft.a\n' "$(MODULE_NAME)"; \
		$(MAKE) -C .. $(LIBFT_ARCHIVE) COMPILE_FLAGS="$(COMPILE_FLAGS)"; \
	fi

../Full_Libft_debug.a:
	@need_build=0; \
	if $(MAKE) -C .. -q $(LIBFT_DEBUG_ARCHIVE) COMPILE_FLAGS="$(COMPILE_FLAGS)"; then \
		printf '        [%s] Full_Libft_debug.a is up to date\n' "$(MODULE_NAME)"; \
	else \
		status=$$?; \
		if [ $$status -eq 1 ]; then \
			need_build=1; \
		else \
			exit $$status; \
		fi; \
	fi; \
	if [ $$need_build -eq 1 ] || [ ! -f $@ ]; then \
		printf '        [%s] Updating Full_Libft_debug.a\n' "$(MODULE_NAME)"; \
		$(MAKE) -C .. $(LIBFT_DEBUG_ARCHIVE) COMPILE_FLAGS="$(COMPILE_FLAGS)"; \
	fi

CLEAN_OBJS := $(shell find $(OBJDIR) $(DEBUG_OBJDIR) -type f -name '*.o' 2>/dev/null)

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
else
    CLEAN_FILES := $(CLEAN_OBJS)
endif

clean:
	@status=0; \
	if ! $(RM) $(CLEAN_FILES); then \
		status=1; \
	fi; \
	if ! $(MAKE) -s -C .. clean COMPILE_FLAGS="$(COMPILE_FLAGS)" >/dev/null; then \
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
	if ! $(RM) $(CLEAN_FILES); then \
		status=1; \
	fi; \
	if ! $(RM) $(TARGET) $(DEBUG_TARGET); then \
		status=1; \
	fi; \
	if ! $(MAKE) -s -C .. fclean COMPILE_FLAGS="$(COMPILE_FLAGS)" >/dev/null; then \
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
