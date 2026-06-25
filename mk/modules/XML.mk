TARGET         := XMLParser.a
DEBUG_TARGET   := XMLParser_debug.a

SRCS := xml_document.cpp \
        xml_node_thread_safety.cpp \
        xml_dom_bridge.cpp \
        xml_serializer.cpp

HEADERS := xml.hpp xml_document.hpp xml_dom_bridge.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
