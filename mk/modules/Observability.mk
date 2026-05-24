TARGET := Observability.a
DEBUG_TARGET := Observability_debug.a

SRCS := observability_task_scheduler_bridge.cpp \
        observability_networking_metrics.cpp \
        observability_game_metrics.cpp

HEADERS := observability_task_scheduler_bridge.hpp \
           observability_networking_metrics.hpp \
           observability_game_metrics.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
