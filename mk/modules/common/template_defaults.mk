MODULE_NAME ?= $(notdir $(CURDIR))
TOTAL_SRCS ?= $(words $(SRCS))

ifeq ($(OS),Windows_NT)
    ifneq ($(LIBFT_POSIX_SHELL),)
        MKDIR ?= mkdir -p
        RM ?= rm -f
        RMDIR ?= rm -rf
    else
        MKDIR ?= mkdir
        RM ?= del /F /Q
        RMDIR ?= rmdir /S /Q
    endif
else
    MKDIR ?= mkdir -p
    RM ?= rm -f
    RMDIR ?= rm -rf
endif

ifdef COMPILE_FLAGS
    CFLAGS := $(COMPILE_FLAGS)
endif

CXX ?= g++
AR ?= ar
ARFLAGS := rcs

BUILD_OBJ_SUFFIX ?= $(BUILD_OUTPUT_SUFFIX)
ifneq ($(findstring -DLIBFT_TEST_BUILD,$(COMPILE_FLAGS)),)
    BUILD_OBJ_SUFFIX := _test$(BUILD_OUTPUT_SUFFIX)
endif

OBJDIR ?= objs$(BUILD_OBJ_SUFFIX)
OBJS ?= $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEPS ?= $(OBJS:.o=.d)
DEBUG_DEPS ?=

CFLAGS ?= -Wall -Wextra -Werror -std=c++17

CLEAN_DIRS ?= $(wildcard objs*)
CLEAN_FILES ?= $(TARGET) $(OBJS) $(DEPS)
ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_FILES))
    CLEAN_DIRS := $(subst /,\\,$(CLEAN_DIRS))
endif
