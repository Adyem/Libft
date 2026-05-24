TOTAL_SRCS := $(words $(SRCS))

MAKEFLAGS += --no-print-directory

ifeq ($(OS),Windows_NT)
    MKDIR = mkdir
    RM    = del /F /Q
else
    MKDIR = mkdir -p
    RM    = rm -f
endif
