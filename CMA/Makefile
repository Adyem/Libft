# Detect OS to set commands for creating directories and removing files
ifeq ($(OS),Windows_NT)
    MKDIR   = mkdir
    RMDIR   = rmdir /S /Q
    RM      = del /F /Q
else
    MKDIR   = mkdir -p
    RMDIR   = rm -rf
    RM      = rm -f
endif

# Compiler and archiver settings.
CXX       := g++
AR        := ar
ARFLAGS   := rcs

# If using cl on Windows, uncomment and adjust the following:
#ifeq ($(OS),Windows_NT)
#    ifeq ($(CXX),cl)
#        AR      := lib
#        ARFLAGS := /OUT:
#    endif
#endif

# Directories
OBJDIR         := objs
DEBUG_OBJDIR   := objs_debug

# Targets
TARGET         := CustomMemoryAllocator.a
DEBUG_TARGET   := CustomMemoryAllocator_debug.a

# Sources and headers
SRCS := cma_calloc.cpp \
        cma_strdup.cpp \
        cma_malloc.cpp \
        cma_free.cpp \
        cma_realloc.cpp \
        cma_itoa.cpp \
        cma_split.cpp \
        cma_strjoin.cpp \
        cma_free_double.cpp \
        cma_utils.cpp \
        cma_cleanup.cpp \
        cma_global_overloads.cpp

HEADERS := CMA.hpp \
           CMA_internal.hpp

# Object files lists
OBJS       := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUG_OBJDIR)/%.o,$(SRCS))

# Compilation flags
CXXFLAGS   := -Wall -Wextra -Werror -g -O0 -std=c++17

# Default target: release version
all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Debug build: add -DDEBUG=1
debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create directories (works cross-platform)
$(OBJDIR) $(DEBUG_OBJDIR):
	$(MKDIR) $@

clean:
	-$(RMDIR) $(OBJDIR)
	-$(RMDIR) $(DEBUG_OBJDIR)

fclean: clean
	-$(RM) $(TARGET) $(DEBUG_TARGET)

re: fclean all

both: all debug

.PHONY: all clean fclean re debug both
