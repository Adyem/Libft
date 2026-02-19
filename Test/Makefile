include ../compiler_flags.mk
TARGET := libft_tests
DEBUG_TARGET := libft_tests_debug
LIBFT_ARCHIVE := Full_Libft.a
LIBFT_DEBUG_ARCHIVE := Full_Libft_debug.a

EFF_SRCS := $(wildcard Efficiency/*.cpp)

ifneq ($(strip $(FILES)),)
SRCS := main.cpp $(FILES)
else
SRCS := main.cpp $(wildcard Test/*.cpp) $(wildcard API/*.cpp)
SRCS := $(filter-out Test/test_template.cpp,$(SRCS))
endif

BASIC_TEST_FILES := \
	Test/test_atoi.cpp \
	Test/test_atol.cpp \
	Test/test_basic_additional_coverage.cpp \
	Test/test_bzero.cpp \
	Test/test_isalnum.cpp \
	Test/test_isalpha.cpp \
	Test/test_isdigit.cpp \
	Test/test_islower.cpp \
	Test/test_isprint.cpp \
	Test/test_isspace.cpp \
	Test/test_isupper.cpp \
	Test/test_libft_utf8.cpp \
	Test/test_memchr.cpp \
	Test/test_memcmp.cpp \
	Test/test_memcpy.cpp \
	Test/test_memmove.cpp \
	Test/test_memset.cpp \
	Test/test_strchr.cpp \
	Test/test_strcmp.cpp \
	Test/test_striteri.cpp \
	Test/test_strlcat.cpp \
	Test/test_strlcpy.cpp \
	Test/test_strlen.cpp \
	Test/test_strmapi.cpp \
	Test/test_strncmp.cpp \
	Test/test_strncpy.cpp \
	Test/test_strnstr.cpp \
	Test/test_strrchr.cpp \
	Test/test_strstr.cpp \
	Test/test_strtok.cpp \
	Test/test_strtol.cpp \
	Test/test_strtoul.cpp \
	Test/test_tolower.cpp \
	Test/test_toupper.cpp \
	Test/test_validate_int.cpp \
	Test/test_wide_char.cpp

CMA_TEST_FILES := \
	Test/test_cma.cpp \
	Test/test_cma_alloc.cpp \
	Test/test_cma_backend.cpp \
	Test/test_cma_block_size.cpp \
	Test/test_cma_global_new.cpp \
	Test/test_cma_limits.cpp \
	Test/test_cma_metadata.cpp \
	Test/test_cma_stats.cpp \
	Test/test_cma_strings.cpp \
	Test/test_scma_accessor.cpp \
	Test/test_scma_accessor_lifecycle.cpp \
	Test/test_scma_accessor_proxy_chain_errors.cpp \
	Test/test_scma_accessor_uninitialized_abort.cpp \
	Test/test_scma_lifecycle.cpp \
	Test/test_scma_memory.cpp \
	Test/test_scma_recursive_mutex.cpp \
	Test/test_scma_thread_safety_controls.cpp \
	Test/test_scma_resize.cpp

GET_NEXT_LINE_TEST_FILES := \
	Test/test_get_next_line.cpp \
	Test/test_get_next_line_strjoin.cpp

CXX       := g++

HAS_OPENSSL_HEADERS := $(shell printf '#include <openssl/ssl.h>\n' | $(CXX) -x c++ -E - >/dev/null 2>&1 && echo 1 || echo 0)
ifneq ($(HAS_OPENSSL_HEADERS),1)
SRCS := $(filter-out Test/test_api_tls_diagnostics.cpp \
                    Test/test_encryption_aead.cpp \
                    Test/test_encryption_aead_copy_move.cpp \
                    Test/test_encryption_hash_algorithms.cpp,$(SRCS))
endif

ifeq ($(HAS_OPENSSL_HEADERS),1)
OPENSSL_LIBS := -lssl -lcrypto
else
OPENSSL_LIBS :=
endif

HAS_SQLITE3_HEADERS := $(shell printf '#include <sqlite3.h>\n' | $(CXX) -x c++ -E - >/dev/null 2>&1 && echo 1 || echo 0)
ifeq ($(HAS_SQLITE3_HEADERS),1)
SQLITE_LIBS := -lsqlite3
else
SQLITE_LIBS :=
endif

TOTAL_SRCS := $(words $(SRCS))

MAKEFLAGS += --no-print-directory

ifeq ($(OS),Windows_NT)
    MKDIR = mkdir
    RM    = del /F /Q
else
    MKDIR = mkdir -p
    RM    = rm -f
endif

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

COMPILE_FLAGS := -Wall -Wextra -Werror -std=c++17 -Wuseless-cast $(OPT_FLAGS) -pthread -Wno-missing-declarations -DLIBFT_TEST_BUILD
CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Libft\"
HAS_X11_LIB := $(shell ldconfig -p 2>/dev/null | grep -q "libX11.so" && echo 1 || echo 0)
HAS_ASOUND_LIB := $(shell ldconfig -p 2>/dev/null | grep -q "libasound.so" && echo 1 || echo 0)
ifeq ($(HAS_X11_LIB),1)
X11_LIBS := -lX11
else
X11_LIBS :=
endif
ifeq ($(HAS_ASOUND_LIB),1)
ASOUND_LIBS := -lasound
else
ASOUND_LIBS :=
endif
LDFLAGS := -Wl,--allow-multiple-definition -lz -ldl $(OPENSSL_LIBS) $(SQLITE_LIBS) $(X11_LIBS) $(ASOUND_LIBS)
export COMPILE_FLAGS

OBJDIR       := objs
DEBUG_OBJDIR := objs_debug

OBJS       := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUG_OBJDIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS) ../Full_Libft.a
	@echo "Linking final executable ($(TARGET))"
	@$(CXX) $(CFLAGS) -o $@ $(OBJS) ../Full_Libft.a $(LDFLAGS)

Basic:
	@echo "Building basic test executable ($(TARGET))"
	@$(MAKE) FILES="$(BASIC_TEST_FILES)" all

CMA:
	@echo "Building CMA test executable ($(TARGET))"
	@$(MAKE) FILES="$(CMA_TEST_FILES)" all

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
	@total=$(TOTAL_SRCS); \
	current=1; \
	for src in $(SRCS); do \
	        if [ "$$src" = "$<" ]; then \
	                break; \
	        fi; \
	        current=$$((current + 1)); \
	done; \
	echo "Compiling test file $$current/$$total";
	@$(CXX) $(CFLAGS) -c $< -o $@

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
$(OBJDIR)/Test/test_atoi.o $(OBJDIR)/Test/test_strjoin_multiple.o $(OBJDIR)/Test/test_to_string.o \
$(OBJDIR)/Test/test_wide_char.o: CFLAGS := $(COMPILE_FLAGS) -DTEST_MODULE=\"Advanced\"
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

$(DEBUG_TARGET): $(DEBUG_OBJS) ../Full_Libft_debug.a
	@echo "Linking final executable ($(DEBUG_TARGET))"
	@$(CXX) $(CFLAGS) -o $@ $(DEBUG_OBJS) ../Full_Libft_debug.a $(LDFLAGS)

$(DEBUG_OBJDIR)/%.o: %.cpp
	@$(MKDIR) $(dir $@)
	@total=$(TOTAL_SRCS); \
	current=1; \
	for src in $(SRCS); do \
	        if [ "$$src" = "$<" ]; then \
	                break; \
	        fi; \
	        current=$$((current + 1)); \
	done; \
	echo "Compiling test file $$current/$$total";
	@$(CXX) $(CFLAGS) -c $< -o $@

../Full_Libft.a:
	@need_build=0; \
	if $(MAKE) -C .. -q $(LIBFT_ARCHIVE) COMPILE_FLAGS="$(COMPILE_FLAGS)"; then \
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
	        $(MAKE) -C .. $(LIBFT_ARCHIVE) COMPILE_FLAGS="$(COMPILE_FLAGS)"; \
	fi

../Full_Libft_debug.a:
	@need_build=0; \
	if $(MAKE) -C .. -q $(LIBFT_DEBUG_ARCHIVE) COMPILE_FLAGS="$(COMPILE_FLAGS)"; then \
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
	        $(MAKE) -C .. $(LIBFT_DEBUG_ARCHIVE) COMPILE_FLAGS="$(COMPILE_FLAGS)"; \
	fi

CLEAN_OBJS := $(shell find $(OBJDIR) $(DEBUG_OBJDIR) -type f -name '*.o' 2>/dev/null)

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
else
    CLEAN_FILES := $(CLEAN_OBJS)
endif

clean:
	@echo "Cleaning test objects"
	@status=0; \
	if ! $(RM) $(CLEAN_FILES); then \
	        status=1; \
	fi; \
	if ! $(MAKE) -s -C .. clean COMPILE_FLAGS="$(COMPILE_FLAGS)"; then \
	        status=1; \
	fi; \
	if [ $$status -ne 0 ]; then \
	        echo "Clean failed"; \
	        exit 1; \
	fi; \
	echo "Clean completed successfully"

fclean:
	@echo "Removing test artifacts"
	@status=0; \
	if ! $(RM) $(CLEAN_FILES); then \
	        status=1; \
	fi; \
	if ! $(RM) $(TARGET) $(DEBUG_TARGET); then \
	        status=1; \
	fi; \
	if ! $(MAKE) -s -C .. fclean COMPILE_FLAGS="$(COMPILE_FLAGS)"; then \
	        status=1; \
	fi; \
	if [ $$status -ne 0 ]; then \
	        echo "Full clean failed"; \
	        exit 1; \
	fi; \
	echo "Full clean completed successfully"

re: fclean all

both: all debug

.PHONY: all clean fclean re debug both build_tests Basic re_Basic CMA re_CMA get_next_line re_get_next_line
