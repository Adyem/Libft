TARGET := Terrain.a
DEBUG_TARGET := Terrain_debug.a

SRCS := terrain_generator.cpp chunk_mesh.cpp

HEADERS := terrain_generator.hpp chunk_mesh.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
