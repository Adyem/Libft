ifeq ($(OS),Windows_NT)
    RM = del /F /Q
else
    RM = rm -f
endif

CXX := g++
