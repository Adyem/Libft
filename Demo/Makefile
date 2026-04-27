include ../compiler_flags.mk

TARGET := demo_dumb_game
MODULE_NAME := $(notdir $(CURDIR))

ifeq ($(OS),Windows_NT)
    RM = del /F /Q
else
    RM = rm -f
endif

CXX := g++
SRCS := demo_main.cpp \
    demo_runtime.cpp \
    demo_render.cpp \
    demo_levels.cpp \
    demo_leaderboard.cpp

UNAME_S := $(shell uname -s)
LINK_FLAGS :=

ifeq ($(OS),Windows_NT)
    LINK_FLAGS += -lgdi32 -luser32
else ifeq ($(UNAME_S),Darwin)
    LINK_FLAGS += -framework Cocoa -framework CoreGraphics -lobjc -lpthread
else
    LINK_FLAGS += -lX11 -lXext -lGL -lpthread
endif

all: ../Full_Libft.a $(TARGET)

$(TARGET): $(SRCS)
	@printf '        [%s] Linking %s\n' "$(MODULE_NAME)" "$(TARGET)"
	@$(CXX) $(COMPILE_FLAGS) $(SRCS) -o $@ ../Full_Libft.a $(LINK_FLAGS)

../Full_Libft.a:
	@$(MAKE) -C .. ../Full_Libft.a

clean:
	@if $(RM) $(TARGET); then \
		printf '        [%s] clean: success\n' "$(MODULE_NAME)"; \
	else \
		printf '        [%s] clean: failed\n' "$(MODULE_NAME)"; \
		exit 1; \
	fi

fclean: clean
	@printf '        [%s] fclean: success\n' "$(MODULE_NAME)"

re: fclean all

.PHONY: all clean fclean re
