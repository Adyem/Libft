include mk/compiler_flags.mk
include mk/build_config.mk

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

re:
	@$(MAKE) fclean
	@$(MAKE) all

$(TARGET): FORCE $(LIBS)
	@printf '\033[1;35m[LIBFT BUILD] Combining %d modules into %s\033[0m\n' $(TOTAL_LIBS) $@
	@$(RM) $@
	@if [ "$$(uname -s)" = "Darwin" ]; then \
		libtool -static -o "$@" $(LIBS); \
	else \
		{ printf 'CREATE %s\n' "$@"; \
		  for lib in $(LIBS); do printf 'ADDLIB %s\n' "$$lib"; done; \
		  printf 'SAVE\nEND\n'; } | $(AR) -M; \
	fi

$(DEBUG_TARGET): FORCE $(DEBUG_LIBS)
	@printf '\033[1;35m[LIBFT BUILD] Combining %d modules into %s\033[0m\n' $(TOTAL_DEBUG_LIBS) $@
	@$(RM) $@
	@if [ "$$(uname -s)" = "Darwin" ]; then \
		libtool -static -o "$@" $(DEBUG_LIBS); \
	else \
		{ printf 'CREATE %s\n' "$@"; \
		  for lib in $(DEBUG_LIBS); do printf 'ADDLIB %s\n' "$$lib"; done; \
		  printf 'SAVE\nEND\n'; } | $(AR) -M; \
	fi

$(TEST_TARGET): FORCE $(TEST_LIBS)
	@printf '\033[1;35m[LIBFT BUILD] Combining %d modules into %s\033[0m\n' $(TOTAL_TEST_LIBS) $@
	@$(RM) $@
	@if [ "$$(uname -s)" = "Darwin" ]; then \
		libtool -static -o "$@" $(TEST_LIBS); \
	else \
		{ printf 'CREATE %s\n' "$@"; \
		  for lib in $(TEST_LIBS); do printf 'ADDLIB %s\n' "$$lib"; done; \
		  printf 'SAVE\nEND\n'; } | $(AR) -M; \
	fi

$(TEST_DEBUG_TARGET): FORCE $(DEBUG_LIBS)
	@printf '\033[1;35m[LIBFT BUILD] Combining %d modules into %s\033[0m\n' $(TOTAL_DEBUG_LIBS) $@
	@$(RM) $@
	@if [ "$$(uname -s)" = "Darwin" ]; then \
		libtool -static -o "$@" $(DEBUG_LIBS); \
	else \
		{ printf 'CREATE %s\n' "$@"; \
		  for lib in $(DEBUG_LIBS); do printf 'ADDLIB %s\n' "$$lib"; done; \
		  printf 'SAVE\nEND\n'; } | $(AR) -M; \
	fi

%_test.a: FORCE
	@module_dir="$(patsubst %/,%,$(dir $@))"; \
	module_target="$(notdir $@)"; \
	module_path="$$module_dir/$$module_target"; \
	progress_index=$$(printf '%s\n' "$(TEST_LIBS)" | tr ' ' '\n' | nl -ba | awk -v target="$$module_path" '$$2==target {print $$1}'); \
	log_file=".libft_build_$$(printf '%s' "$$module_path" | tr '/.' '__')_$$$$.log"; \
	{ \
		printf '\033[1;35m[LIBFT BUILD] (%d/%d) Building %s\033[0m\n' "$$progress_index" "$(TOTAL_TEST_LIBS)" "$$module_path"; \
		$(MAKE) -C $$module_dir $$module_target $(SUBMAKE_OVERRIDES) TARGET="$$module_target" COMPILE_FLAGS="$(COMPILE_FLAGS) -DLIBFT_TEST_BUILD"; \
	} > "$$log_file" 2>&1; \
	status=$$?; \
	while ! mkdir .libft_output_lock 2>/dev/null; do sleep 0.02; done; \
	cat "$$log_file"; \
	rmdir .libft_output_lock; \
	$(RM) "$$log_file"; \
	if [ $$status -ne 0 ]; then exit $$status; fi

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
                log_file=".libft_build_$$(printf '%s' "$$module_path" | tr '/.' '__')_$$$$.log"; \
                { \
                        printf '\033[1;35m[LIBFT BUILD] (%d/%d) Building %s\033[0m\n' "$$progress_index" "$(TOTAL_LIBS)" "$$module_path"; \
                        $(MAKE) -C $$module_dir $$module_target $(SUBMAKE_OVERRIDES); \
                } > "$$log_file" 2>&1; \
                status=$$?; \
                while ! mkdir .libft_output_lock 2>/dev/null; do sleep 0.02; done; \
                cat "$$log_file"; \
                rmdir .libft_output_lock; \
                $(RM) "$$log_file"; \
                if [ $$status -ne 0 ]; then exit $$status; fi; \
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
                log_file=".libft_build_$$(printf '%s' "$$module_path" | tr '/.' '__')_$$$$.log"; \
                { \
                        printf '\033[1;35m[LIBFT BUILD] (%d/%d) Building %s\033[0m\n' "$$progress_index" "$(TOTAL_DEBUG_LIBS)" "$$module_path"; \
                        $(MAKE) -C $$module_dir $$module_target $(SUBMAKE_OVERRIDES); \
                } > "$$log_file" 2>&1; \
                status=$$?; \
                while ! mkdir .libft_output_lock 2>/dev/null; do sleep 0.02; done; \
                cat "$$log_file"; \
                rmdir .libft_output_lock; \
                $(RM) "$$log_file"; \
                if [ $$status -ne 0 ]; then exit $$status; fi; \
        fi

clean:
	@status=0; \
	for dir in $(SUBDIRS); do \
		if ! $(MAKE) -C $$dir clean $(SUBMAKE_OVERRIDES); then \
			status=1; \
		fi; \
	done; \
	if ! $(MAKE) -C Demo clean $(SUBMAKE_OVERRIDES) LIBFT_PARENT_CLEAN=1; then \
		status=1; \
	fi; \
	if ! $(MAKE) -C Test clean $(SUBMAKE_OVERRIDES) LIBFT_PARENT_CLEAN=1; then \
		status=1; \
	fi; \
	if ! $(RMDIR) $(TEMP_DIRS); then \
		status=1; \
	fi; \
	if ! $(RM) $(TARGET) $(DEBUG_TARGET) $(TEST_TARGET) $(TEST_DEBUG_TARGET) $(OUTPUT_LOGS); then \
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
	if ! $(MAKE) -C Demo fclean $(SUBMAKE_OVERRIDES) LIBFT_PARENT_CLEAN=1; then \
		status=1; \
	fi; \
	if ! $(MAKE) -C Test fclean $(SUBMAKE_OVERRIDES) LIBFT_PARENT_CLEAN=1; then \
		status=1; \
	fi; \
	if ! $(RMDIR) $(TEMP_DIRS); then \
		status=1; \
	fi; \
	if ! $(RM) $(TARGET) $(DEBUG_TARGET) $(TEST_TARGET) $(TEST_DEBUG_TARGET) $(OUTPUT_LOGS); then \
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
