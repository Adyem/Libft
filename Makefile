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
                -Wfloat-equal -Wodr -Wuseless-cast -Wzero-as-null-pointer-constant \
				-Wmaybe-uninitialized $(OPT_FLAGS) $(SANITIZER_FLAGS)

export COMPILE_FLAGS
export SANITIZER_FLAGS
export SANITIZERS

CXX             := g++
AR              := ar
ARFLAGS         := rcs
CLANG_FORMAT   ?= clang-format

MAKEFLAGS      += --no-print-directory

SUBMAKE_OVERRIDES := -j1

ifeq ($(OS),Windows_NT)
    MKDIR  = mkdir
    RM     = del /F /Q
    RMDIR  = rmdir /S /Q
else
    MKDIR  = mkdir -p
    RM     = rm -f
    RMDIR  = rm -rf
endif

SUBDIRS :=  Libft \
            Compatebility \
            Errno \
            CMA \
            SCMA \
            GetNextLine \
            DUMB \
            Math \
            Geometry \
            System_utils \
            Printf \
            ReadLine \
            PThread \
            CPP_class \
            Config \
            CrossProcess \
            Compression \
            Encryption \
            RNG \
            JSon \
            YAML \
            File \
            HTML \
            Time \
            XML \
            Storage \
            Networking \
            API \
            Observability \
            Logger \
            Parser \
            Game

LIB_BASES := \
  Libft/LibFT \
  Compatebility/Compatebility \
  Errno/errno \
  CMA/CustomMemoryAllocator \
  SCMA/SCMA \
  GetNextLine/GetNextLine \
  DUMB/dumb \
  Math/Math \
  Geometry/geometry \
  System_utils/System_utils \
  Printf/Printf \
  ReadLine/ReadLine \
  PThread/PThread \
  CPP_class/CPP_class \
  Config/config \
  CrossProcess/CrossProcess \
  Compression/compression \
  Encryption/encryption \
  RNG/RNG \
  JSon/JSon \
  YAML/YAML  \
  File/file \
  HTML/HTMLParser \
  Time/time \
  XML/XMLParser \
  Storage/storage \
  Networking/networking \
  API/API \
  Observability/Observability \
  Logger/Logger \
  Parser/parser \
  Game/Game

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
	@need_build=0; \
	if $(MAKE) -C Test -q all $(SUBMAKE_OVERRIDES); then \
		printf '\033[1;35m[LIBFT CHECK] Test suite is up to date\033[0m\n'; \
	else \
		status=$$?; \
		if [ $$status -eq 1 ]; then \
			need_build=1; \
		else \
			exit $$status; \
		fi; \
	fi; \
	if [ $$need_build -eq 1 ]; then \
		printf '\033[1;35m[LIBFT BUILD] Updating test suite\033[0m\n'; \
		$(MAKE) -C Test all $(SUBMAKE_OVERRIDES); \
	fi

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
	@module_dir="$(patsubst %/,%,$(dir $@))"; \
	module_target="$(notdir $@)"; \
	need_build=0; \
	if $(MAKE) -C $$module_dir -q $$module_target $(SUBMAKE_OVERRIDES); then \
	        :; \
	else \
	        status=$$?; \
	        if [ $$status -eq 1 ]; then \
	                need_build=1; \
	        else \
	                exit $$status; \
	        fi; \
	fi; \
        if [ $$need_build -eq 1 ] || [ ! -f $@ ]; then \
                module_path="$$module_dir/$$module_target"; \
                progress_index=$$(printf '%s\n' "$(LIBS)" | tr ' ' '\n' | nl -ba | awk -v target="$$module_path" '$$2==target {print $$1}'); \
                printf '\033[1;35m[LIBFT BUILD] (%d/%d) Building %s\033[0m\n' "$$progress_index" "$(TOTAL_LIBS)" "$$module_path"; \
                $(RM) $$module_dir/objs/*.o; \
                $(RM) $$module_dir/$$module_target; \
                $(MAKE) -C $$module_dir $$module_target $(SUBMAKE_OVERRIDES); \
        fi

%_debug.a:
	@module_dir="$(patsubst %/,%,$(dir $@))"; \
	module_target="$(notdir $@)"; \
	need_build=0; \
	if $(MAKE) -C $$module_dir -q $$module_target $(SUBMAKE_OVERRIDES); then \
	        :; \
	else \
	        status=$$?; \
	        if [ $$status -eq 1 ]; then \
	                need_build=1; \
	        else \
	                exit $$status; \
	        fi; \
	fi; \
        if [ $$need_build -eq 1 ] || [ ! -f $@ ]; then \
                module_path="$$module_dir/$$module_target"; \
                progress_index=$$(printf '%s\n' "$(DEBUG_LIBS)" | tr ' ' '\n' | nl -ba | awk -v target="$$module_path" '$$2==target {print $$1}'); \
                printf '\033[1;35m[LIBFT BUILD] (%d/%d) Building %s\033[0m\n' "$$progress_index" "$(TOTAL_DEBUG_LIBS)" "$$module_path"; \
                $(RM) $$module_dir/objs_debug/*.o; \
                $(RM) $$module_dir/$$module_target; \
                $(MAKE) -C $$module_dir $$module_target $(SUBMAKE_OVERRIDES); \
        fi

clean:
	@status=0; \
	for dir in $(SUBDIRS); do \
		if ! $(MAKE) -C $$dir clean $(SUBMAKE_OVERRIDES); then \
			status=1; \
		fi; \
	done; \
	if ! $(RM) $(TARGET) $(DEBUG_TARGET); then \
		status=1; \
	fi; \
	if [ $$status -eq 0 ]; then \
		printf '\033[1;35m[LIBFT CLEAN] Success\033[0m\n'; \
	else \
		printf '\033[1;35m[LIBFT CLEAN] Failed\033[0m\n'; \
		exit 1; \
	fi

fclean:
	@status=0; \
	for dir in $(SUBDIRS); do \
		if ! $(MAKE) -C $$dir fclean $(SUBMAKE_OVERRIDES); then \
			status=1; \
		fi; \
	done; \
	if ! $(RM) $(TARGET) $(DEBUG_TARGET); then \
		status=1; \
	fi; \
	if [ $$status -eq 0 ]; then \
		printf '\033[1;35m[LIBFT FCLEAN] Success\033[0m\n'; \
	else \
		printf '\033[1;35m[LIBFT FCLEAN] Failed\033[0m\n'; \
		exit 1; \
	fi

.PHONY: all debug both re clean fclean tests format sanitize-clean \
        asan asan-tests ubsan ubsan-tests asan-ubsan asan-ubsan-tests
