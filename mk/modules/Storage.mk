TARGET         := storage.a
DEBUG_TARGET   := storage_debug.a

SRCS := storage_kv_store_constructor.cpp \
        storage_kv_store_entry.cpp \
        storage_kv_store_encryption.cpp \
        storage_kv_store_operations.cpp

HEADERS := kv_store.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
