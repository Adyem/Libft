TARGET := JSon.a
DEBUG_TARGET := JSon_debug.a

SRCS :=         json_parsing.cpp \
                json_create_item.cpp \
                json_reader.cpp \
                json_stream_reader.cpp \
                json_stream_writer.cpp \
                json_writer.cpp \
                json_utils.cpp \
                json_document.cpp \
                json_schema.cpp \
                json_schema_evolution.cpp \
                json_dom_bridge.cpp \
                json_serializer.cpp \
                json_thread_safety.cpp

HEADERS := json.hpp document.hpp json_schema.hpp json_schema_evolution.hpp json_stream_reader.hpp json_stream_writer.hpp json_stream_events.hpp json_dom_bridge.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
