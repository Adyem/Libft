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
endif

COMPILE_FLAGS = -Wall -Werror -Wextra -std=c++17 -Wmissing-declarations \
                -Wold-style-cast -Wshadow -Wconversion -Wformat=2 -Wundef \
                -Wfloat-equal -Wconversion -Wodr -Wuseless-cast \
                -Wzero-as-null-pointer-constant -Wmaybe-uninitialized $(OPT_FLAGS) \
                $(SANITIZER_FLAGS)

export COMPILE_FLAGS
export SANITIZER_FLAGS
export SANITIZERS

CXX             := g++
AR              := ar
ARFLAGS         := rcs
CLANG_FORMAT   ?= clang-format

MAKEFLAGS      += --no-print-directory

SUBMAKE_OVERRIDES :=

ifeq ($(OS),Windows_NT)
    MKDIR  = mkdir
    RM     = del /F /Q
    RMDIR  = rmdir /S /Q
else
    MKDIR  = mkdir -p
    RM     = rm -f
    RMDIR  = rm -rf
endif

SUBDIRS :=  CMA \
            GetNextLine \
            Libft \
            Math \
            Geometry \
            Logger \
            System_utils \
            Printf \
            ReadLine \
            PThread \
            CPP_class \
            Errno \
            Config \
            Networking \
            API \
             Compatebility \
             Compression Encryption RNG JSon YAML File HTML Game Time XML Storage

LIB_BASES := \
  CMA/CustomMemoryAllocator \
  GetNextLine/GetNextLine \
  Libft/LibFT \
  Math/Math \
  Geometry/geometry \
  Logger/Logger \
  System_utils/System_utils \
  Printf/Printf \
  ReadLine/ReadLine \
  PThread/PThread \
  CPP_class/CPP_class \
  Errno/errno \
  Config/config \
  Networking/networking \
  API/API \
  Compatebility/Compatebility \
  Compression/compression \
  Encryption/encryption \
  RNG/RNG \
  JSon/JSon \
    YAML/YAML  \
  File/file \
  HTML/HTMLParser \
  Game/Game \
  Time/time \
  XML/XMLParser \
  Storage/storage

LIBS       := $(addsuffix .a, $(LIB_BASES))
DEBUG_LIBS := $(addsuffix _debug.a, $(LIB_BASES))
TOTAL_LIBS := $(words $(LIBS))
TOTAL_DEBUG_LIBS := $(words $(DEBUG_LIBS))

TARGET        := Full_Libft.a
DEBUG_TARGET  := Full_Libft_debug.a

all: $(TARGET)

debug: $(DEBUG_TARGET)

both: all debug

tests: $(TARGET)
	$(MAKE) -C Test $(SUBMAKE_OVERRIDES)

format:
	@if ! command -v $(CLANG_FORMAT) >/dev/null 2>&1; then \
		echo "Error: clang-format not found."; \
		exit 1; \
	fi
	@files="$$(git ls-files '*.cpp' '*.hpp' '*.ipp')"; \
	if [ -n "$$files" ]; then \
		printf '%s\n' "$$files" | tr '\n' '\0' | xargs -0 $(CLANG_FORMAT) --style=file -i; \
	else \
		echo "No source files to format."; \
	fi

sanitize-clean:
	$(MAKE) clean

asan: sanitize-clean
	$(MAKE) SANITIZERS=address all

asan-tests: sanitize-clean
	$(MAKE) SANITIZERS=address tests

ubsan: sanitize-clean
	$(MAKE) SANITIZERS=undefined all

ubsan-tests: sanitize-clean
	$(MAKE) SANITIZERS=undefined tests

asan-ubsan: sanitize-clean
	$(MAKE) SANITIZERS="address undefined" all

asan-ubsan-tests: sanitize-clean
	$(MAKE) SANITIZERS="address undefined" tests

re: fclean all

define EXTRACT
cd temp_objs && $(AR) x ../$1 && cd ..;
endef

$(TARGET): $(LIBS)
	@printf '\033[1;35m[LIBFT BUILD] Combining %d modules into %s\033[0m\n' $(TOTAL_LIBS) $@
	@$(RM) $@
	@$(MKDIR) temp_objs
	@$(foreach lib,$(LIBS),$(call EXTRACT,$(lib)))
	@$(AR) $(ARFLAGS) $@ temp_objs/*.o
	@$(RMDIR) temp_objs

$(DEBUG_TARGET): $(DEBUG_LIBS)
	@printf '\033[1;35m[LIBFT BUILD] Combining %d modules into %s\033[0m\n' $(TOTAL_DEBUG_LIBS) $@
	@$(RM) $@
	@$(MKDIR) temp_objs
	@$(foreach lib,$(DEBUG_LIBS),$(call EXTRACT,$(lib)))
	@$(AR) $(ARFLAGS) $@ temp_objs/*.o
	@$(RMDIR) temp_objs

%.a:
	@$(MAKE) -C $(dir $@) $(SUBMAKE_OVERRIDES)
	@built=0; \
	for lib in $(LIBS); do \
		if [ -f $$lib ]; then \
			built=$$((built + 1)); \
		fi; \
	done; \
	printf '\033[1;35m[LIBFT PROGRESS] Modules completed: %d/%d\033[0m\n' $$built $(TOTAL_LIBS)

%_debug.a:
	@$(MAKE) -C $(dir $@) debug $(SUBMAKE_OVERRIDES)
	@built=0; \
	for lib in $(DEBUG_LIBS); do \
		if [ -f $$lib ]; then \
			built=$$((built + 1)); \
		fi; \
	done; \
	printf '\033[1;35m[LIBFT PROGRESS] Debug modules completed: %d/%d\033[0m\n' $$built $(TOTAL_DEBUG_LIBS)

clean:
	@$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) clean;)
	@$(RM) $(TARGET) $(DEBUG_TARGET)

fclean:
	@$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) fclean;)
	@$(RM) $(TARGET) $(DEBUG_TARGET)

.PHONY: all debug both re clean fclean tests format sanitize-clean \
        asan asan-tests ubsan ubsan-tests asan-ubsan asan-ubsan-tests
