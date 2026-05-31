TARGET := Application.a
DEBUG_TARGET := Application_debug.a

SRCS := application_auth_service.cpp

HEADERS := application.hpp application_auth_service.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
