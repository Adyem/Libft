include ../mk/compiler_flags.mk
include ../mk/test/config.mk

all: $(TARGET)

.PHONY: FORCE
FORCE:
	@true

$(TARGET): $(OBJS) ../$(LIBFT_ARCHIVE)
	@printf "Compiled test files (%d files)\n" $(TOTAL_SRCS)
	@echo "Linking final executable ($(TARGET))"
	@$(CXX) $(CFLAGS) -o $@ $(OBJS) ../$(LIBFT_ARCHIVE) $(LDFLAGS)

Basic:
	@echo "Building basic test executable ($(TARGET))"
	@$(MAKE) FILES="$(BASIC_TEST_FILES)" all

CMA:
	@echo "Building CMA test executable ($(TARGET))"
	@$(MAKE) FILES="$(CMA_TEST_FILES)" all

Efficiency:
	@echo "Building efficiency test executable ($(EFFICIENCY_TARGET)) with OPT_LEVEL=$(EFFICIENCY_OPT_LEVEL)"
	@$(MAKE) $(EFFICIENCY_TARGET) OPT_LEVEL=$(EFFICIENCY_OPT_LEVEL)
	@echo "Efficiency executable ready; run ./$(EFFICIENCY_TARGET) manually if desired."

run_Efficiency:
	@$(MAKE) $(EFFICIENCY_TARGET) OPT_LEVEL=$(EFFICIENCY_OPT_LEVEL)
	@./$(EFFICIENCY_TARGET)

single_test:
	@if [ -z "$(TEST)" ]; then \
		echo "Usage: make single_test TEST=Test/test_name.cpp"; \
		exit 1; \
	fi
	@python3 tools/build_single_test.py "$(TEST)"

run_single_test: single_test

get_next_line:
	@echo "Building get_next_line test executable ($(TARGET))"
	@$(MAKE) FILES="$(GET_NEXT_LINE_TEST_FILES)" all

re_get_next_line: fclean get_next_line

re_Basic: fclean Basic

re_CMA: fclean CMA

build_tests: all
	@echo "Test executable ready; run ./$(TARGET) manually if desired."

$(OBJDIR)/%.o: %.cpp
	@$(MKDIR) $(dir $@)
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR)/Test/test_promise.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_task_scheduler.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"PThread\"
$(OBJDIR)/Test/test_pthread_thread.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"PThread\"
$(OBJDIR)/Test/test_pthread_thread_thread_safety.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"PThread\"
$(OBJDIR)/Test/test_pthread_blocking_queue_thread_safety.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"PThread\"
$(OBJDIR)/Test/test_pthread_task_scheduler_thread_safety.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"PThread\"
$(OBJDIR)/Test/test_pthread_lock_tracking.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"PThread\"
$(OBJDIR)/Test/test_template_future_thread_safety.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_future_lifecycle.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_pool_thread_safety.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_stack_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_queue_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_bitset_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_function_copy_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_vector_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_matrix_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_priority_queue_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_pair_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_variant_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_pool_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_set_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_unique_ptr_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_shared_ptr_copy_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_future_copy_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_tuple_move_mutex.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_circular_buffer_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_unordered_map_copy_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_queue_lifecycle_abort.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_graph_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_graph_move_errors.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_deque_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_deque_move_errors.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_iterator_copy_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_unordered_map_allocation_failure_paths.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_template_trie_copy_move_deleted.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Template\"
$(OBJDIR)/Test/test_advanced_memory.o $(OBJDIR)/Test/test_advanced_numeric.o $(OBJDIR)/Test/test_advanced_strings.o \
$(OBJDIR)/Test/test_strjoin_multiple.o $(OBJDIR)/Test/test_to_string.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Advanced\"
$(OBJDIR)/Test/test_basic_atoi.o $(OBJDIR)/Test/test_basic_wstrlen.o \
$(OBJDIR)/Test/test_basic_utf8.o $(OBJDIR)/Test/test_basic_validate_int.o \
$(OBJDIR)/Test/test_basic_additional_coverage.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Basic\"
$(OBJDIR)/Test/test_networking.o $(OBJDIR)/Test/test_http_server.o $(OBJDIR)/Test/test_websocket.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Networking\"
$(OBJDIR)/Test/test_networking_quic.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Networking\"
$(OBJDIR)/Test/test_logger.o $(OBJDIR)/Test/test_logger_async.o $(OBJDIR)/Test/test_logger_network.o \
$(OBJDIR)/Test/test_logger_file.o $(OBJDIR)/Test/test_logger_control.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Logger\"
$(OBJDIR)/Test/test_json_validate.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"JSon\"
$(OBJDIR)/Test/test_yaml.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"YAML\"
$(OBJDIR)/Test/test_rng.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"RNG\"
$(OBJDIR)/Test/test_rng_statistical_quality.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"RNG\"
$(OBJDIR)/Test/test_rng_distribution_coverage.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"RNG\"
$(OBJDIR)/Test/test_rng_vectorized_sampling.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"RNG\"
$(OBJDIR)/Test/test_rng_stream.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"RNG\"
$(OBJDIR)/Test/test_linear_algebra.o $(OBJDIR)/Test/test_quaternion.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Math\"
$(OBJDIR)/Test/test_string_view.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"String\"
$(OBJDIR)/Test/test_pathfinding.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_data_catalog.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_progress_tracker.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_economy_table.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_economy_table_copy_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_economy_records.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_economy_record_setters.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_economy_helpers.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_economy_table_setters.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_behavior_profiles.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_behavior_records.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_behavior_record_setters.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_behavior_helpers.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_behavior_table_copy_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_data_catalog_records.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_data_catalog_record_setters.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_dialogue_repository.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_dialogue_records.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_dialogue_record_setters.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_dialogue_helpers.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_dialogue_table_copy_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_dialogue_table_setters.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_world_registry.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_world_registry_records.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_world_registry_record_setters.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_world_registry_table_copy_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_game_world_registry_table_setters.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Game\"
$(OBJDIR)/Test/test_encryption_key.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Encryption\"
$(OBJDIR)/Test/test_encryption_secure_wipe.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Encryption\"
$(OBJDIR)/Test/test_encryption_key_management.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Encryption\"
$(OBJDIR)/Test/test_encryption_aead_copy_move.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Encryption\"
$(OBJDIR)/Test/test_xml_copy_move_deleted.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"XML\"

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS) ../$(LIBFT_DEBUG_ARCHIVE)
	@printf "Compiled debug test files (%d files)\n" $(TOTAL_SRCS)
	@echo "Linking final executable ($(DEBUG_TARGET))"
	@$(CXX) $(CFLAGS) -o $@ $(DEBUG_OBJS) ../$(LIBFT_DEBUG_ARCHIVE) $(LDFLAGS)

$(DEBUG_OBJDIR)/%.o: %.cpp
	@$(MKDIR) $(dir $@)
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@

$(EFFICIENCY_TARGET): $(EFFICIENCY_OBJS) ../$(EFFICIENCY_LIBFT_ARCHIVE)
	@printf "Compiled efficiency files (%d files)\n" $(EFF_TOTAL_SRCS)
	@echo "Linking final executable ($(EFFICIENCY_TARGET))"
	@$(CXX) $(EFFICIENCY_CFLAGS) -o $@ $(EFFICIENCY_OBJS) ../$(EFFICIENCY_LIBFT_ARCHIVE) $(LDFLAGS)

$(EFFICIENCY_OBJDIR)/%.o: %.cpp
	@$(MKDIR) $(dir $@)
	@$(CXX) $(EFFICIENCY_CFLAGS) -MMD -MP -c $< -o $@

../$(EFFICIENCY_LIBFT_ARCHIVE):
	@need_build=0; \
	if $(MAKE) -C .. -q $(EFFICIENCY_LIBFT_ARCHIVE) COMPILE_FLAGS="$(EFFICIENCY_COMPILE_FLAGS)" \
		OPT_LEVEL=$(OPT_LEVEL); then \
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
	        echo "Building $(EFFICIENCY_LIBFT_ARCHIVE)"; \
	        $(MAKE) -C .. $(EFFICIENCY_LIBFT_ARCHIVE) COMPILE_FLAGS="$(EFFICIENCY_COMPILE_FLAGS)" \
			OPT_LEVEL=$(OPT_LEVEL); \
	fi

../$(LIBFT_ARCHIVE):
	@need_build=0; \
	if $(MAKE) -C .. -q $(LIBFT_ARCHIVE) COMPILE_FLAGS="$(COMPILE_FLAGS)" \
		OPT_LEVEL=$(OPT_LEVEL); then \
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
	        echo "Building $(LIBFT_ARCHIVE)"; \
	        $(MAKE) -C .. $(LIBFT_ARCHIVE) COMPILE_FLAGS="$(COMPILE_FLAGS)" \
			OPT_LEVEL=$(OPT_LEVEL); \
	fi

../$(LIBFT_DEBUG_ARCHIVE):
	@need_build=0; \
	if $(MAKE) -C .. -q $(LIBFT_DEBUG_ARCHIVE) COMPILE_FLAGS="$(COMPILE_FLAGS)" \
		OPT_LEVEL=$(OPT_LEVEL); then \
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
	        echo "Building $(LIBFT_DEBUG_ARCHIVE)"; \
	        $(MAKE) -C .. $(LIBFT_DEBUG_ARCHIVE) COMPILE_FLAGS="$(COMPILE_FLAGS)" \
			OPT_LEVEL=$(OPT_LEVEL); \
	fi

clean:
	@echo "Cleaning test objects"
	@status=0; \
	if ! $(RM) $(CLEAN_FILES); then \
	        status=1; \
	fi; \
	if ! $(RMDIR) $(CLEAN_DIRS); then \
	        status=1; \
	fi; \
	if [ $$status -ne 0 ]; then \
	        echo "Clean failed"; \
	        exit 1; \
	fi; \
	echo "Clean completed successfully"

fclean: clean
	@echo "Removing test artifacts"
	@status=0; \
	if ! $(RM) $(TARGET) $(DEBUG_TARGET) $(EFFICIENCY_TARGET); then \
		status=1; \
	fi; \
	if [ -z "$(LIBFT_PARENT_CLEAN)" ]; then \
		if ! $(MAKE) -C .. fclean; then \
			status=1; \
		fi; \
	fi; \
	if [ $$status -ne 0 ]; then \
		echo "Full clean failed"; \
		exit 1; \
	fi; \
	echo "Full clean completed successfully"

re:
	@$(MAKE) fclean
	@$(MAKE) all

both: all debug

-include $(DEPS) $(DEBUG_DEPS) $(EFFICIENCY_DEPS)

.PHONY: all clean fclean re debug both build_tests Basic re_Basic CMA re_CMA Efficiency run_Efficiency get_next_line re_get_next_line FORCE

print_test_compile_flags:
	@printf '%s\n' "$(COMPILE_FLAGS)"

print_test_ldflags:
	@printf '%s\n' "$(LDFLAGS)"

print_test_objdir:
	@printf '%s\n' "$(OBJDIR)"

FORCE:
