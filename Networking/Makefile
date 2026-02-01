TARGET := networking.a
DEBUG_TARGET := networking_debug.a

SRCS := networking_socket_class.cpp \
        networking_send_utils.cpp \
        networking.cpp \
        networking_dns_resolver.cpp \
        networking_setup_server.cpp \
        networking_setup_client.cpp \
        networking_setup_udp.cpp \
        networking_udp_event_loop.cpp \
        networking_socket_wrapper_functions.cpp \
        socket_handle.cpp \
        networking_ssl_wrapper.cpp \
        networking_tls_aead.cpp \
        networking_quic_experimental.cpp \
        networking_nonblocking.cpp \
        networking_event_loop.cpp \
        http_client.cpp \
        http2_client.cpp \
        http_server.cpp \
        websocket_client.cpp \
        websocket_server.cpp \
        networking_socket_config_thread_safety.cpp

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

MODULE_NAME := $(notdir $(CURDIR))
TOTAL_SRCS := $(words $(SRCS))

HEADERS := socket_class.hpp \
           networking.hpp \
           udp_socket.hpp \
           ssl_wrapper.hpp \
           networking_tls_aead.hpp \
           networking_quic_experimental.hpp \
           http_client.hpp \
           http2_client.hpp \
           http_server.hpp \
           websocket_client.hpp \
           websocket_server.hpp \
           socket_handle.hpp \

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

CFLAGS   ?= -Wall -Wextra -Werror -g -O0 -std=c++17 -Wuseless-cast

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	@$(CXX) $(CFLAGS) -c $< -o $@
	@count=$$(find $(OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
	    printf '        [%s] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s Debug] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
	@$(CXX) $(CFLAGS) -c $< -o $@
	@count=$$(find $(DEBUG_OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
	    printf '        [%s Debug] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

$(OBJDIR) $(DEBUG_OBJDIR):
	@$(MKDIR) $@

CLEAN_OBJS := $(wildcard $(OBJDIR)/*.o) $(wildcard $(DEBUG_OBJDIR)/*.o)

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
else
    CLEAN_FILES := $(CLEAN_OBJS)
endif

clean:
	@if $(RM) $(CLEAN_FILES); then \
		printf '        [%s] clean: success\n' "$(MODULE_NAME)"; \
	else \
		printf '        [%s] clean: failed\n' "$(MODULE_NAME)"; \
		exit 1; \
	fi

fclean:
	@status=0; \
	if ! $(RM) $(CLEAN_FILES); then \
		status=1; \
	fi; \
	if ! $(RM) $(TARGET) $(DEBUG_TARGET); then \
		status=1; \
	fi; \
	if [ $$status -eq 0 ]; then \
		printf '        [%s] fclean: success\n' "$(MODULE_NAME)"; \
	else \
		printf '        [%s] fclean: failed\n' "$(MODULE_NAME)"; \
		exit 1; \
	fi

re: fclean all

both: all debug

.PHONY: all clean fclean re debug both

