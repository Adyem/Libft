include ../compiler_flags.mk
MODULE_NAME := $(notdir $(CURDIR))
SRCS := template_basic_types.cpp \
        template_optional.cpp \
        template_variant.cpp \
        template_compile_algorithm.cpp \
        template_compile_bitset.cpp \
        template_compile_cancellation.cpp \
        template_compile_circular_buffer.cpp \
        template_compile_deque.cpp \
        template_compile_event_emitter.cpp \
        template_compile_function.cpp \
        template_compile_future.cpp \
        template_compile_graph.cpp \
        template_compile_iterator.cpp \
        template_compile_map.cpp \
        template_compile_math.cpp \
        template_compile_matrix.cpp \
        template_compile_optional.cpp \
        template_compile_pool.cpp \
        template_compile_priority_queue.cpp \
        template_compile_promise.cpp \
        template_compile_queue.cpp \
        template_compile_set.cpp \
        template_compile_shared_ptr.cpp \
        template_compile_stack.cpp \
        template_compile_string_view.cpp \
        template_compile_thread_pool.cpp \
        template_compile_trie.cpp \
        template_compile_tuple.cpp \
        template_compile_unique_ptr.cpp \
        template_compile_unordered_map.cpp \
        template_compile_variant.cpp \
        template_compile_vector.cpp
TOTAL_SRCS := $(words $(SRCS))
TARGET := template_basic_types.a
OBJDIR := objs
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
CXX := g++
AR := ar
ARFLAGS := rcs

CFLAGS ?= $(COMPILE_FLAGS)
ifeq ($(strip $(CFLAGS)),)
    CFLAGS := -Wall -Wextra -Werror -std=c++17
endif

ifeq ($(OS),Windows_NT)
    MKDIR = mkdir
    RM = del /F /Q
else
    MKDIR = mkdir -p
    RM = rm -f
endif

CLEAN_FILES := $(TARGET) $(OBJS)
ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_FILES))
endif

all: $(TARGET)
	@if $(RM) $(CLEAN_FILES); then \
        printf '        [%s] template verification passed (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS); \
    else \
        printf '        [%s] cleanup failed\n' "$(MODULE_NAME)"; \
        exit 1; \
    fi

$(TARGET): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	@$(CXX) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	@$(MKDIR) $@

clean:
	@if $(RM) $(CLEAN_FILES); then \
        printf '        [%s] clean: success\n' "$(MODULE_NAME)"; \
    else \
        printf '        [%s] clean: failed\n' "$(MODULE_NAME)"; \
        exit 1; \
    fi

fclean: clean

re: fclean all

.PHONY: all clean fclean re
