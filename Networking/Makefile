TARGET := networking.a
DEBUG_TARGET := networking_debug.a

SRCS := networking_socket_class.cpp \
        networking.cpp \
        networking_setup_server.cpp \
        networking_setup_client.cpp \
        networking_setup_udp.cpp \
        networking_socket_wrapper_functions.cpp \
        networking_ssl_wrapper.cpp \
        networking_nonblocking.cpp \
        networking_event_loop.cpp \
        http_client.cpp \
        http_server.cpp \
        websocket_client.cpp \
        websocket_server.cpp

ifeq ($(OS),Windows_NT)
    SRCS += networking_select.cpp
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        SRCS += networking_epoll.cpp
    else ifeq ($(UNAME_S),Darwin)
        SRCS += networking_kqueue.cpp
    else ifeq ($(UNAME_S),FreeBSD)
        SRCS += networking_kqueue.cpp
    else ifeq ($(UNAME_S),NetBSD)
        SRCS += networking_kqueue.cpp
    else ifeq ($(UNAME_S),OpenBSD)
        SRCS += networking_kqueue.cpp
    else
        SRCS += networking_select.cpp
    endif
endif

HEADERS := socket_class.hpp \
           networking.hpp \
           udp_socket.hpp \
           ssl_wrapper.hpp \
           http_client.hpp \
           http_server.hpp \
           websocket_client.hpp \
           websocket_server.hpp \

ifeq ($(OS),Windows_NT)
    MKDIR   = mkdir
    RM      = del /F /Q
else
    MKDIR   = mkdir -p
    RM      = rm -f
endif

ifdef COMPILE_FLAGS
    CFLAGS := $(COMPILE_FLAGS)
endif

CXX       := g++
AR        := ar
ARFLAGS   := rcs

OBJDIR         := objs
DEBUG_OBJDIR   := objs_debug

OBJS       := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUG_OBJDIR)/%.o,$(SRCS))

CFLAGS   ?= -Wall -Wextra -Werror -g -O0 -std=c++17

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	$(CXX) $(CFLAGS) -c $< -o $@

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(DEBUG_OBJDIR):
	$(MKDIR) $@

CLEAN_OBJS := $(wildcard $(OBJDIR)/*.o) $(wildcard $(DEBUG_OBJDIR)/*.o)

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
else
    CLEAN_FILES := $(CLEAN_OBJS)
endif

clean:
	-$(RM) $(CLEAN_FILES)

fclean: clean
	-$(RM) $(TARGET) $(DEBUG_TARGET)

re: fclean all

both: all debug

.PHONY: all clean fclean re debug both
