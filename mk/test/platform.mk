TOTAL_SRCS := $(words $(SRCS))

MAKEFLAGS += --no-print-directory

LIBFT_TEST_PARALLEL_JOBS := $(filter -j%,$(MAKEFLAGS))
LIBFT_TEST_JOBSERVER := $(filter --jobserver-auth=%,$(MAKEFLAGS))
LIBFT_TEST_BATCH_OUTPUT := 0
ifneq ($(LIBFT_TEST_JOBSERVER),)
    LIBFT_TEST_BATCH_OUTPUT := 1
else ifneq ($(LIBFT_TEST_PARALLEL_JOBS),)
    ifneq ($(LIBFT_TEST_PARALLEL_JOBS),-j1)
        LIBFT_TEST_BATCH_OUTPUT := 1
    endif
endif

LIBFT_TEST_OUTPUT_LOCK = .libft_test_output_lock
LIBFT_TEST_OUTPUT_LOGS = .libft_test_build_*.log

ifeq ($(OS),Windows_NT)
    MKDIR = mkdir
    RM    = del /F /Q
    RMDIR = rmdir /S /Q
else
    MKDIR = mkdir -p
    RM    = rm -f
    RMDIR = rm -rf
endif
