ifeq ($(OS),Windows_NT)
    ifeq ($(strip $(LIBFT_POSIX_SHELL)),)
        ifneq ($(findstring bash,$(SHELL)),)
            LIBFT_POSIX_SHELL := 1
        else ifneq ($(findstring sh,$(SHELL)),)
            LIBFT_POSIX_SHELL := 1
        endif
    endif
    ifneq ($(LIBFT_POSIX_SHELL),)
        RM = rm -f
        RMDIR = rm -rf
    else
        RM = del /F /Q
        RMDIR = rmdir /S /Q
    endif
else
    RM = rm -f
    RMDIR = rm -rf
endif

CXX := g++
