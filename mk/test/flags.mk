OPT_LEVEL ?= 0

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

COMPILE_FLAGS := -Wall -Wextra -Werror -std=c++17 -Wuseless-cast $(OPT_FLAGS) -pthread -Wno-missing-declarations -fcf-protection=none -DLIBFT_TEST_BUILD -DLIBFT_INTERNAL_HEADERS
CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Libft\"
EFFICIENCY_COMPILE_FLAGS := $(filter-out -DLIBFT_TEST_BUILD,$(COMPILE_FLAGS)) -DFT_EFFICIENCY_BUILD
EFFICIENCY_CFLAGS := $(EFFICIENCY_COMPILE_FLAGS) -DTEST_MODULE=\"Libft\"

LDFLAGS := -Wl,--allow-multiple-definition -lz -ldl $(OPENSSL_LIBS) $(SQLITE_LIBS) $(X11_LIBS) $(XEXT_LIBS) $(ASOUND_LIBS)
export COMPILE_FLAGS
