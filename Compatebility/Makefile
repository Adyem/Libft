include ../compiler_flags.mk
TARGET := Compatebility.a
DEBUG_TARGET := Compatebility_debug.a

.RECIPEPREFIX := >

SRCS := Compatebility_file_io.cpp Compatebility_file_ops.cpp Compatebility_file_watch.cpp Compatebility_file_dir.cpp Compatebility_file_path.cpp Compatebility_rng.cpp Compatebility_readline.cpp Compatebility_pthread.cpp Compatebility_system.cpp Compatebility_write.cpp Compatebility_syslog.cpp Compatebility_networking.cpp Compatebility_time.cpp Compatebility_cross_process_posix.cpp Compatebility_cross_process_windows.cpp
MM_SRCS :=
HEADERS := compatebility_internal.hpp \
        compatebility_cross_process.hpp
ifeq ($(OS),Windows_NT)
MKDIR   = mkdir
RM      = del /F /Q
else
MKDIR   = mkdir -p
RM      = rm -f
endif

ifeq ($(OS),Windows_NT)
SRCS += Compatebility_dumb_render_win32.cpp
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
MM_SRCS += Compatebility_dumb_render_platform_macos.mm
MMFLAGS += -x objective-c++
else
SRCS += Compatebility_dumb_render_linux_x11.cpp
endif
endif

ifdef COMPILE_FLAGS
CFLAGS := $(COMPILE_FLAGS)
endif

CXX       := g++
AR        := ar
ARFLAGS   := rcs

OBJDIR         := objs
DEBUG_OBJDIR   := objs_debug

CFLAGS   ?= -Wall -Wextra -Werror -g -O0 -std=c++17 -Wuseless-cast
MMFLAGS  := $(CFLAGS)

CPP_OBJS       := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
MM_OBJS        := $(patsubst %.mm,$(OBJDIR)/%.o,$(MM_SRCS))
OBJS           := $(CPP_OBJS) $(MM_OBJS)

DEBUG_CPP_OBJS := $(patsubst %.cpp,$(DEBUG_OBJDIR)/%.o,$(SRCS))
DEBUG_MM_OBJS  := $(patsubst %.mm,$(DEBUG_OBJDIR)/%.o,$(MM_SRCS))
DEBUG_OBJS     := $(DEBUG_CPP_OBJS) $(DEBUG_MM_OBJS)

all: $(TARGET)

$(TARGET): $(OBJS)
>@$(AR) $(ARFLAGS) $@ $^
>@printf '        [%s] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
>@$(CXX) $(CFLAGS) -c $< -o $@
>@count=$$(find $(OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
>    printf '        [%s] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.mm $(HEADERS) | $(OBJDIR)
>@$(CXX) $(MMFLAGS) -c $< -o $@
>@count=$$(find $(OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
>    printf '        [%s] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
>@$(AR) $(ARFLAGS) $@ $^
>@printf '        [%s Debug] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
>@$(CXX) $(CFLAGS) -c $< -o $@
>@count=$$(find $(DEBUG_OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
>    printf '        [%s Debug] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

$(OBJDIR) $(DEBUG_OBJDIR):
>@$(MKDIR) $@

CLEAN_OBJS := $(wildcard $(OBJDIR)/*.o) $(wildcard $(DEBUG_OBJDIR)/*.o)

MODULE_NAME := $(notdir $(CURDIR))
TOTAL_SRCS := $(words $(SRCS))

ifeq ($(OS),Windows_NT)
CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
else
CLEAN_FILES := $(CLEAN_OBJS)
endif

clean:
>@if $(RM) $(CLEAN_FILES); then \
>    printf '        [%s] clean: success\n' "$(MODULE_NAME)"; \
>else \
>    printf '        [%s] clean: failed\n' "$(MODULE_NAME)"; \
>    exit 1; \
>fi

fclean:
>@status=0; \
>if ! $(RM) $(CLEAN_FILES); then \
>    status=1; \
>fi; \
>if ! $(RM) $(TARGET) $(DEBUG_TARGET); then \
>    status=1; \
>fi; \
>if [ $$status -eq 0 ]; then \
>    printf '        [%s] fclean: success\n' "$(MODULE_NAME)"; \
>else \
>    printf '        [%s] fclean: failed\n' "$(MODULE_NAME)"; \
>    exit 1; \
>fi

re: fclean all

both: all debug

.PHONY: all clean fclean re debug both
