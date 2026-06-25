TARGET := CSV.a
DEBUG_TARGET := CSV_debug.a

SRCS :=         csv.cpp

HEADERS := csv.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
