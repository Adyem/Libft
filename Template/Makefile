MODULE_NAME := $(notdir $(CURDIR))
SRCS := template_basic_types.cpp \
        template_optional.cpp \
        template_variant.cpp \
        template_module_compile.cpp
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
