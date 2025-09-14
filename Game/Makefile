TARGET := Game.a
DEBUG_TARGET := Game_debug.a

SRCS := game_map3d.cpp \
	game_character_constructor.cpp \
	game_character_getters_setters.cpp \
	game_character_add_remove.cpp \
	game_character_misc.cpp \
	game_character_save_load.cpp \
	game_quest.cpp \
	game_achievement.cpp \
	game_reputation.cpp \
	game_buff.cpp \
	game_debuff.cpp \
	game_skill.cpp \
	game_upgrade.cpp \
	game_event.cpp \
	game_event_scheduler.cpp \
	game_world.cpp \
	game_server.cpp \
	game_item.cpp \
	game_inventory.cpp \
	game_equipment.cpp \
	game_save.cpp \
	game_load.cpp \
	game_experience_table.cpp \
	game_pathfinding.cpp \
	game_crafting.cpp \
        game_state.cpp

HEADERS := game_map3d.hpp \
	game_character.hpp \
	game_quest.hpp \
	game_achievement.hpp \
	game_reputation.hpp \
	game_buff.hpp \
	game_debuff.hpp \
	game_skill.hpp \
	game_upgrade.hpp \
	game_event.hpp \
	game_event_scheduler.hpp \
	game_world.hpp \
	game_server.hpp \
	game_item.hpp \
	game_inventory.hpp \
	game_equipment.hpp \
	game_experience_table.hpp \
	game_pathfinding.hpp \
	game_crafting.hpp \
        game_state.hpp

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
