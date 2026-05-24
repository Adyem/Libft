TARGET := geometry.a
DEBUG_TARGET := geometry_debug.a

SRCS := geometry_aabb.cpp \
        geometry_3d.cpp \
        geometry_collision.cpp \
        geometry_circle.cpp \
        geometry_lock_tracker.cpp \
        geometry_sphere.cpp

HEADERS := geometry.hpp aabb.hpp circle.hpp geometry_3d.hpp geometry_lock_tracker.hpp sphere.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
