include ../compiler_flags.mk
TARGET := Game.a
DEBUG_TARGET := Game_debug.a

SRCS := game_map3d.cpp \
        game_character_constructor.cpp \
        game_character_getters_setters.cpp \
        game_character_add_remove.cpp \
        game_character_misc.cpp \
        game_character_metrics.cpp \
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
        game_event_scheduler_telemetry.cpp \
        game_world.cpp \
        game_world_replay.cpp \
        game_server.cpp \
        game_item.cpp \
	game_inventory.cpp \
	game_equipment.cpp \
	game_save.cpp \
	game_load.cpp \
	game_experience_table.cpp \
        game_resistance.cpp \
        game_pathfinding.cpp \
        game_crafting.cpp \
        game_data_catalog.cpp \
        game_dialogue_line.cpp \
        game_dialogue_script.cpp \
        game_dialogue_table.cpp \
        game_behavior_action.cpp \
        game_behavior_profile.cpp \
        game_behavior_table.cpp \
        game_region_definition.cpp \
        game_world_region.cpp \
        game_world_registry.cpp \
        game_price_definition.cpp \
        game_rarity_band.cpp \
        game_vendor_profile.cpp \
        game_currency_rate.cpp \
        game_economy_table.cpp \
        game_progress_tracker.cpp \
        game_hooks.cpp \
        game_state.cpp \
        game_behavior_tree.cpp \
        game_scripting_bridge.cpp

MODULE_NAME := $(notdir $(CURDIR))
TOTAL_SRCS := $(words $(SRCS))

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
        game_event_scheduler_telemetry.hpp \
        game_world.hpp \
        game_world_replay.hpp \
        game_server.hpp \
	game_item.hpp \
	game_inventory.hpp \
	game_equipment.hpp \
        game_experience_table.hpp \
        game_resistance.hpp \
        game_pathfinding.hpp \
        game_crafting.hpp \
        game_data_catalog.hpp \
        ft_dialogue_line.hpp \
        ft_dialogue_script.hpp \
        game_dialogue_table.hpp \
        ft_behavior_action.hpp \
        ft_behavior_profile.hpp \
        game_behavior_table.hpp \
        ft_region_definition.hpp \
        ft_world_region.hpp \
        game_world_registry.hpp \
        ft_price_definition.hpp \
        ft_rarity_band.hpp \
        ft_vendor_profile.hpp \
        ft_currency_rate.hpp \
        game_economy_table.hpp \
        game_progress_tracker.hpp \
        game_hooks.hpp \
        game_state.hpp \
        game_behavior_tree.hpp \
        game_scripting_bridge.hpp

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

