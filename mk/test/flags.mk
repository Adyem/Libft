OPT_LEVEL ?= 0
UNAME_S := $(shell uname -s)

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

COMPILE_FLAGS := -Wall -Wextra -Werror -std=c++17 $(OPT_FLAGS) -pthread \
                -Wno-missing-declarations -Wshadow -Wformat=2 -Wundef \
                -Wfloat-equal -Wodr \
                -DLIBFT_TEST_BUILD -DLIBFT_INTERNAL_HEADERS \
                $(SANITIZER_FLAGS)

ifeq ($(UNAME_S),Darwin)
COMPILE_FLAGS += -Wno-format-nonliteral -Wno-tautological-compare
else
COMPILE_FLAGS += -Wuseless-cast -Wold-style-cast -Wconversion \
                -Wzero-as-null-pointer-constant -Wmaybe-uninitialized \
                -fcf-protection=none
endif

ifeq ($(OS),Windows_NT)
COMPILE_FLAGS += -I..
endif

CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Libft\"
EFFICIENCY_COMPILE_FLAGS := $(filter-out -DLIBFT_TEST_BUILD,$(COMPILE_FLAGS)) -DFT_EFFICIENCY_BUILD
EFFICIENCY_CFLAGS := $(EFFICIENCY_COMPILE_FLAGS) -DTEST_MODULE=\"Libft\"

ifeq ($(UNAME_S),Darwin)
LDFLAGS := -lz -framework Cocoa -framework CoreGraphics -framework QuartzCore \
           -framework AudioToolbox -lobjc -lpthread $(OPENSSL_LIBS) $(SQLITE_LIBS)
else ifeq ($(OS),Windows_NT)
LDFLAGS := -Wl,--allow-multiple-definition -lz -lws2_32 -lgdi32 -lwinmm -ldbghelp \
           $(OPENSSL_LIBS) $(SQLITE_LIBS)
else
LDFLAGS := -Wl,--allow-multiple-definition -rdynamic -lz -ldl \
           $(OPENSSL_LIBS) $(SQLITE_LIBS) $(X11_LIBS) $(XEXT_LIBS) $(XI_LIBS) $(ASOUND_LIBS)
endif

export COMPILE_FLAGS
