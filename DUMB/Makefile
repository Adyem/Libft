NAME = dumb.a

SRCS = dumb_io.cpp \
       dumb_sound.cpp \
       dumb_render.cpp \
       dumb_sound_clip.cpp \
       dumb_console.cpp

OBJS_DIR = objs
OBJS = $(addprefix $(OBJS_DIR)/, $(SRCS:.cpp=.o))

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -I.. -g -std=c++17

ifeq ($(OS), Windows_NT)
    SRCS += dumb_sound_win32.cpp
    CXXFLAGS += -D_WIN32
    LIBS = -lwinmm
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S), Darwin)
        SRCS += dumb_sound_macos_coreaudio.cpp dumb_render_platform_macos.mm
        CXXFLAGS += -x objective-c++
        FRAMEWORKS = -framework Cocoa -framework AudioToolbox
    else
        SRCS += dumb_sound_linux_alsa.cpp dumb_render_linux_x11.cpp
        LIBS = -lX11 -lasound
    endif
endif

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)

$(OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJS_DIR)/%.o: %.mm
	@mkdir -p $(OBJS_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all