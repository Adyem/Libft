include compiler_flags.mk

DEMO_OPT_LEVEL ?= 3

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

SUBDIRS := Modules/Basic \
           Modules/Advanced \
           Modules/Compatebility \
           Modules/Errno \
           Modules/CMA \
           Modules/SCMA \
           Modules/GetNextLine \
           Modules/DUMB \
           Modules/Math \
           Modules/Geometry \
           Modules/System_utils \
           Modules/Printf \
           Modules/ReadLine \
           Modules/PThread \
           Modules/CPP_class \
           Modules/Template \
           Modules/Buffer \
           Modules/CLI \
           Modules/Command \
           Modules/Config \
           Modules/CrossProcess \
           Modules/Compression \
           Modules/Encryption \
           Modules/Encoding \
           Modules/RNG \
           Modules/JSon \
           Modules/YAML \
           Modules/File \
           Modules/HTML \
           Modules/Time \
           Modules/Filesystem \
           Modules/XML \
           Modules/Storage \
           Modules/Networking \
           Modules/API \
           Modules/Observability \
           Modules/Logger \
           Modules/Parser \
           Modules/Game

LIB_BASES := \
  Modules/Basic/Basic \
  Modules/Advanced/Advanced \
  Modules/Compatebility/Compatebility \
  Modules/Errno/errno \
  Modules/CMA/CustomMemoryAllocator \
  Modules/SCMA/SCMA \
  Modules/GetNextLine/GetNextLine \
  Modules/DUMB/dumb \
  Modules/Math/Math \
  Modules/Geometry/geometry \
  Modules/System_utils/System_utils \
  Modules/Printf/Printf \
  Modules/ReadLine/ReadLine \
  Modules/PThread/PThread \
  Modules/CPP_class/CPP_class \
  Modules/Buffer/buffer \
  Modules/CLI/cli \
  Modules/Command/command \
  Modules/Config/config \
  Modules/CrossProcess/CrossProcess \
  Modules/Compression/compression \
  Modules/Encryption/encryption \
  Modules/Encoding/encoding \
  Modules/RNG/RNG \
  Modules/JSon/JSon \
  Modules/YAML/YAML  \
  Modules/File/file \
  Modules/HTML/HTMLParser \
  Modules/Time/time \
  Modules/Filesystem/filesystem \
  Modules/XML/XMLParser \
  Modules/Storage/storage \
  Modules/Networking/networking \
  Modules/API/API \
  Modules/Observability/Observability \
  Modules/Logger/Logger \
  Modules/Parser/parser \
  Modules/Game/Game

LIBS       := $(addsuffix .a, $(LIB_BASES))
DEBUG_LIBS := $(addsuffix _debug.a, $(LIB_BASES))
TEST_LIBS  := $(addsuffix _test.a, $(LIB_BASES))
TOTAL_LIBS := $(words $(LIBS))
TOTAL_DEBUG_LIBS := $(words $(DEBUG_LIBS))
TOTAL_TEST_LIBS := $(words $(TEST_LIBS))

TARGET        := Full_Libft.a
DEBUG_TARGET  := Full_Libft_debug.a
TEST_TARGET   := Full_Libft_test.a
TEST_DEBUG_TARGET := Full_Libft_test_debug.a

CPP_CLASS_LIB := Modules/CPP_class/CPP_class.a

all: $(TARGET) template

debug: $(DEBUG_TARGET)

both: all debug

demo:
	@printf '\033[1;35m[LIBFT BUILD] Building Demo module\033[0m\n'
	@$(MAKE) -C Demo all -B $(SUBMAKE_OVERRIDES) OPT_LEVEL=$(DEMO_OPT_LEVEL)

template: $(CPP_CLASS_LIB)
	@printf '\033[1;35m[LIBFT BUILD] Running Template verification\033[0m\n'
	@$(MAKE) -C Modules/Template all $(SUBMAKE_OVERRIDES)

tests:
	@printf '\033[1;35m[LIBFT BUILD] Ensuring test archive %s\033[0m\n' "$(TEST_TARGET)"
	@$(MAKE) $(TEST_TARGET) $(SUBMAKE_OVERRIDES) COMPILE_FLAGS="$(COMPILE_FLAGS) -DLIBFT_TEST_BUILD"
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

$(TARGET): $(LIBS)
	@printf '\033[1;35m[LIBFT BUILD] Combining %d modules into %s\033[0m\n' $(TOTAL_LIBS) $@
	@$(RM) $@
	@$(RMDIR) temp_objs
	@$(MKDIR) temp_objs
	@temp_index=0; \
	for lib in $(LIBS); do \
		temp_index=$$((temp_index + 1)); \
		$(MKDIR) temp_objs/$$temp_index; \
		cd temp_objs/$$temp_index && $(AR) x ../../$$lib && cd ../..; \
	done
	@find temp_objs -type f -name '*.o' -exec $(AR) $(ARFLAGS) $@ {} +
	@find temp_objs -mindepth 1 -exec rm -rf {} +
	@$(RMDIR) temp_objs

$(DEBUG_TARGET): $(DEBUG_LIBS)
	@printf '\033[1;35m[LIBFT BUILD] Combining %d modules into %s\033[0m\n' $(TOTAL_DEBUG_LIBS) $@
	@$(RM) $@
	@$(RMDIR) temp_objs
	@$(MKDIR) temp_objs
	@temp_index=0; \
	for lib in $(DEBUG_LIBS); do \
		temp_index=$$((temp_index + 1)); \
		$(MKDIR) temp_objs/$$temp_index; \
		cd temp_objs/$$temp_index && $(AR) x ../../$$lib && cd ../..; \
	done
	@find temp_objs -type f -name '*.o' -exec $(AR) $(ARFLAGS) $@ {} +
	@find temp_objs -mindepth 1 -exec rm -rf {} +
	@$(RMDIR) temp_objs

$(TEST_TARGET): $(TEST_LIBS)
	@printf '\033[1;35m[LIBFT BUILD] Combining %d modules into %s\033[0m\n' $(TOTAL_TEST_LIBS) $@
	@$(RM) $@
	@$(RMDIR) temp_objs_test
	@$(MKDIR) temp_objs_test
	@temp_index=0; \
	for lib in $(TEST_LIBS); do \
		temp_index=$$((temp_index + 1)); \
		$(MKDIR) temp_objs_test/$$temp_index; \
		cd temp_objs_test/$$temp_index && $(AR) x ../../$$lib && cd ../..; \
	done
	@find temp_objs_test -type f -name '*.o' -exec $(AR) $(ARFLAGS) $@ {} +
	@find temp_objs_test -mindepth 1 -exec rm -rf {} +
	@$(RMDIR) temp_objs_test

$(TEST_DEBUG_TARGET): $(DEBUG_LIBS)
	@printf '\033[1;35m[LIBFT BUILD] Combining %d modules into %s\033[0m\n' $(TOTAL_DEBUG_LIBS) $@
	@$(RM) $@
	@$(RMDIR) temp_objs_test
	@$(MKDIR) temp_objs_test
	@temp_index=0; \
	for lib in $(DEBUG_LIBS); do \
		temp_index=$$((temp_index + 1)); \
		$(MKDIR) temp_objs_test/$$temp_index; \
		cd temp_objs_test/$$temp_index && $(AR) x ../../$$lib && cd ../..; \
	done
	@find temp_objs_test -type f -name '*.o' -exec $(AR) $(ARFLAGS) $@ {} +
	@find temp_objs_test -mindepth 1 -exec rm -rf {} +
	@$(RMDIR) temp_objs_test

%.a: FORCE
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
                $(MAKE) -C $$module_dir $$module_target $(SUBMAKE_OVERRIDES); \
        fi

%_debug.a: FORCE
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
                $(MAKE) -C $$module_dir $$module_target $(SUBMAKE_OVERRIDES); \
        fi

%_test.a: FORCE
	@module_dir="$(patsubst %/,%,$(dir $@))"; \
	module_target="$(notdir $@)"; \
	need_build=0; \
	if $(MAKE) -C $$module_dir -q $$module_target $(SUBMAKE_OVERRIDES) TARGET="$$module_target" COMPILE_FLAGS="$(COMPILE_FLAGS) -DLIBFT_TEST_BUILD"; then \
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
                progress_index=$$(printf '%s\n' "$(TEST_LIBS)" | tr ' ' '\n' | nl -ba | awk -v target="$$module_path" '$$2==target {print $$1}'); \
                printf '\033[1;35m[LIBFT BUILD] (%d/%d) Building %s\033[0m\n' "$$progress_index" "$(TOTAL_TEST_LIBS)" "$$module_path"; \
                $(MAKE) -C $$module_dir $$module_target $(SUBMAKE_OVERRIDES) TARGET="$$module_target" COMPILE_FLAGS="$(COMPILE_FLAGS) -DLIBFT_TEST_BUILD"; \
        fi

clean:
	@status=0; \
	for dir in $(SUBDIRS); do \
		if ! $(MAKE) -C $$dir clean $(SUBMAKE_OVERRIDES); then \
			status=1; \
		fi; \
	done; \
	if ! $(MAKE) -C Demo clean $(SUBMAKE_OVERRIDES); then \
		status=1; \
	fi; \
	if ! $(RM) $(TARGET) $(DEBUG_TARGET) $(TEST_TARGET) $(TEST_DEBUG_TARGET); then \
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
	if ! $(MAKE) -C Demo fclean $(SUBMAKE_OVERRIDES); then \
		status=1; \
	fi; \
	if ! $(RM) $(TARGET) $(DEBUG_TARGET) $(TEST_TARGET) $(TEST_DEBUG_TARGET); then \
		status=1; \
	fi; \
	if [ $$status -eq 0 ]; then \
		printf '\033[1;35m[LIBFT FCLEAN] Success\033[0m\n'; \
	else \
		printf '\033[1;35m[LIBFT FCLEAN] Failed\033[0m\n'; \
		exit 1; \
	fi

.PHONY: all debug both template demo re clean fclean tests format sanitize-clean \
        asan asan-tests ubsan ubsan-tests asan-ubsan asan-ubsan-tests FORCE

FORCE:
