TARGET         := HTMLParser.a
DEBUG_TARGET   := HTMLParser_debug.a

SRCS := html_node.cpp html_writer.cpp html_cleanup.cpp html_search.cpp html_string_writer.cpp html_document.cpp

HEADERS := html_parser.hpp html_document.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
