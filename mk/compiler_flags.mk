ifndef COMPILER_FLAGS_INCLUDED
COMPILER_FLAGS_INCLUDED := 1

OPT_LEVEL ?= 0
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

SANITIZERS ?=
SANITIZER_FLAGS :=
SANITIZER_SUFFIX :=
ifneq ($(strip $(SANITIZERS)),)
    SANITIZER_SELECTION := $(sort $(SANITIZERS))
    UNSUPPORTED_SANITIZERS := $(filter-out address undefined,$(SANITIZER_SELECTION))
    ifneq ($(UNSUPPORTED_SANITIZERS),)
        $(error Unsupported SANITIZERS: $(UNSUPPORTED_SANITIZERS))
    endif
    ifneq ($(filter address,$(SANITIZER_SELECTION)),)
        SANITIZER_FLAGS += -fsanitize=address
    endif
    ifneq ($(filter undefined,$(SANITIZER_SELECTION)),)
        SANITIZER_FLAGS += -fsanitize=undefined
    endif
    SANITIZER_FLAGS += -fno-omit-frame-pointer
    EMPTY :=
    SPACE := $(EMPTY) $(EMPTY)
    SANITIZER_SUFFIX := _san_$(subst $(SPACE),_,$(SANITIZER_SELECTION))
endif

BUILD_OUTPUT_SUFFIX := _opt$(OPT_LEVEL)$(SANITIZER_SUFFIX)

COMPILE_FLAGS ?= -Wall -Werror -Wextra -std=c++17 -Wmissing-declarations \
                -Wshadow -Wformat=2 -Wundef \
                -Wfloat-equal -Wodr \
                -DLIBFT_INTERNAL_HEADERS \
                $(OPT_FLAGS) $(SANITIZER_FLAGS)

ifneq ($(shell uname -s 2>/dev/null),Darwin)
    COMPILE_FLAGS += -Wold-style-cast -Wconversion -Wuseless-cast \
        -Wzero-as-null-pointer-constant -Wmaybe-uninitialized
else
    COMPILE_FLAGS += -Wno-format-nonliteral -Wno-tautological-compare
endif

export BUILD_OUTPUT_SUFFIX
export COMPILE_FLAGS
export SANITIZER_FLAGS
export SANITIZERS
endif
