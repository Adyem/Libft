TARGET         := filesystem.a
DEBUG_TARGET   := filesystem_debug.a

SRCS := filesystem_path.cpp \
        filesystem_temp.cpp \
        filesystem_atomic_write.cpp \
        filesystem_walk.cpp

HEADERS := filesystem.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
