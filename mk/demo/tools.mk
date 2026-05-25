ifeq ($(OS),Windows_NT)
    RM = del /F /Q
    RMDIR = rmdir /S /Q
else
    RM = rm -f
    RMDIR = rm -rf
endif

CXX := g++
