ifeq ($(OS),Windows_NT)
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
