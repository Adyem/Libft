HAS_OPENSSL_HEADERS := $(shell printf "\#include <openssl/ssl.h>\n" | $(CXX) -x c++ -E - >/dev/null 2>&1 && echo 1 || echo 0)
ifneq ($(HAS_OPENSSL_HEADERS),1)
SRCS := $(filter-out Test/test_api_tls_diagnostics.cpp \
                    Test/test_encryption_aead.cpp \
                    Test/test_encryption_aead_copy_move.cpp \
                    Test/test_encryption_hash_algorithms.cpp,$(SRCS))
endif

HAS_OPENSSL_LIBS := $(shell printf 'int main(void){return 0;}\n' | $(CXX) -x c++ -o /tmp/libft_test_openssl_check_$$$$_tmp - -lssl -lcrypto >/dev/null 2>&1 && echo 1 || echo 0)
ifeq ($(and $(HAS_OPENSSL_HEADERS),$(HAS_OPENSSL_LIBS)),1)
OPENSSL_LIBS := -lssl -lcrypto
else
OPENSSL_LIBS :=
endif

HAS_SQLITE3_HEADERS := $(shell printf "\#include <sqlite3.h>\n" | $(CXX) -x c++ -E - >/dev/null 2>&1 && echo 1 || echo 0)
HAS_SQLITE3_LIBS := $(shell printf 'int main(void){return 0;}\n' | $(CXX) -x c++ -o /tmp/libft_test_sqlite3_check_$$$$_tmp - -lsqlite3 >/dev/null 2>&1 && echo 1 || echo 0)
ifeq ($(and $(HAS_SQLITE3_HEADERS),$(HAS_SQLITE3_LIBS)),1)
SQLITE_LIBS := -lsqlite3
else
SQLITE_LIBS :=
endif

ifeq ($(OS),Windows_NT)
ZLIB_LIBS := -lz
else
UNAME_S := $(shell uname -s 2>/dev/null)
ifeq ($(UNAME_S),Linux)
HAS_ZLIB_HEADERS := $(shell printf "\#include <zlib.h>\n" | $(CXX) -x c++ -E - >/dev/null 2>&1 && echo 1 || echo 0)
HAS_ZLIB_LIBS := $(shell printf 'int main(void){return 0;}\n' | $(CXX) -x c++ -o /tmp/libft_test_zlib_check_$$$$_tmp - -lz >/dev/null 2>&1 && echo 1 || echo 0)
ifeq ($(and $(HAS_ZLIB_HEADERS),$(HAS_ZLIB_LIBS)),1)
ZLIB_LIBS := -lz
else
ZLIB_LIBS :=
SRCS := $(filter-out Test/test_compression.cpp \
                    Test/test_compression_memory.cpp \
                    Test/test_compression_stream.cpp \
                    Test/test_websocket_compression.cpp,$(SRCS))
endif
else
ZLIB_LIBS := -lz
endif
endif

HAS_X11_HEADERS := $(shell printf "\#include <X11/Xlib.h>\n\#include <X11/Xatom.h>\n" | $(CXX) -x c++ -E - >/dev/null 2>&1 && echo 1 || echo 0)
HAS_X11_LIB := $(shell printf 'int main(void){return 0;}\n' | $(CXX) -x c++ -o /tmp/libft_test_x11_check_$$$$_tmp - -lX11 >/dev/null 2>&1 && echo 1 || echo 0)
HAS_ASOUND_LIB := $(shell ldconfig -p 2>/dev/null | grep -q "libasound.so" && echo 1 || echo 0)
ifeq ($(and $(HAS_X11_HEADERS),$(HAS_X11_LIB)),1)
X11_LIBS := -lX11
else
X11_LIBS :=
endif

HAS_XEXT_HEADERS := $(shell printf "\#include <X11/extensions/Xdbe.h>\n" | $(CXX) -x c++ -E - >/dev/null 2>&1 && echo 1 || echo 0)
HAS_XEXT_LIB := $(shell printf 'int main(void){return 0;}\n' | $(CXX) -x c++ -o /tmp/libft_test_xext_check_$$$$_tmp - -lXext >/dev/null 2>&1 && echo 1 || echo 0)
HAS_XI_HEADERS := $(shell printf "\#include <X11/extensions/XInput2.h>\n" | $(CXX) -x c++ -E - >/dev/null 2>&1 && echo 1 || echo 0)
HAS_XI_LIB := $(shell printf 'int main(void){return 0;}\n' | $(CXX) -x c++ -o /tmp/libft_test_xi_check_$$$$_tmp - -lXi >/dev/null 2>&1 && echo 1 || echo 0)
HAS_GL_HEADERS := $(shell printf "\#include <GL/gl.h>\n\#include <GL/glext.h>\n\#include <GL/glx.h>\n" | $(CXX) -x c++ -E - >/dev/null 2>&1 && echo 1 || echo 0)
HAS_GL_LIB := $(shell printf 'int main(void){return 0;}\n' | $(CXX) -x c++ -o /tmp/libft_test_gl_check_$$$$_tmp - -lGL >/dev/null 2>&1 && echo 1 || echo 0)
ifeq ($(HAS_ASOUND_LIB),1)
ASOUND_LIBS := -lasound
else
ASOUND_LIBS :=
endif

ifeq ($(and $(HAS_XEXT_HEADERS),$(HAS_XEXT_LIB)),1)
XEXT_LIBS := -lXext
else
XEXT_LIBS :=
endif

ifeq ($(and $(HAS_XI_HEADERS),$(HAS_XI_LIB)),1)
XI_LIBS := -lXi
else
XI_LIBS :=
endif

ifeq ($(and $(HAS_GL_HEADERS),$(HAS_GL_LIB)),1)
GL_LIBS := -lGL
else
GL_LIBS :=
endif
