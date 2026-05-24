TARGET := encryption.a
DEBUG_TARGET := encryption_debug.a

SRCS := Encryption_basic_encryption.cpp \
        Encryption_aead.cpp \
        Encryption_key.cpp \
        Encryption_aes.cpp \
        Encryption_rsa.cpp \
        Encryption_secure_wipe.cpp \
        Encryption_hardware_acceleration.cpp \
        Encryption_sha256.cpp \
        Encryption_sha3.cpp \
        Encryption_blake2.cpp \
        Encryption_sha1.cpp \
        Encryption_hmac_sha256.cpp \
        Encryption_key_management.cpp

HEADERS := encryption.hpp \
        encryption_internal.hpp \
        encryption_aead_context.hpp

include $(dir $(lastword $(MAKEFILE_LIST)))common/module_defaults.mk
