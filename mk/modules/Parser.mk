TARGET := parser.a
DEBUG_TARGET := parser_debug.a

SRCS :=         parser_document_backend.cpp \
                parser_dom_node.cpp \
                parser_dom_document.cpp \
                parser_dom_validation_report.cpp \
                parser_dom_schema.cpp \
                parser_dom_find_path.cpp

HEADERS := document_backend.hpp dom.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
