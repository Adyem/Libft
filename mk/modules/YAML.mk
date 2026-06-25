TARGET := YAML.a
DEBUG_TARGET := YAML_debug.a

SRCS := yaml_reader.cpp \
        yaml_value.cpp \
        yaml_reader_utils.cpp \
        yaml_writer.cpp \
        yaml_dom_bridge.cpp \
        yaml_serializer.cpp

HEADERS := yaml.hpp yaml_dom_bridge.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
