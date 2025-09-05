CXX      := g++
AR       := ar
ARFLAGS  := rcs
CXXFLAGS := -Wall -Wextra -Werror -g -O0 -std=c++17

ifeq ($(OS),Windows_NT)
    MKDIR  = mkdir
    RM     = del /F /Q
    RMDIR  = rmdir /S /Q
else
    MKDIR  = mkdir -p
    RM     = rm -f
    RMDIR  = rm -rf
endif

SUBDIRS :=  CMA \
	    GetNextLine \
	    Libft \
	    Logger \
	    Printf \
	    ReadLine \
	    PThread \
	    CPP_class \
	    Errno \
	    Config \
	    Networking \
	    API

ifeq ($(OS),Windows_NT)
SUBDIRS += Windows
else
SUBDIRS += Linux
endif

SUBDIRS += encryption RNG JSon file HTML Game

LIB_BASES := \
  CMA/CustomMemoryAllocator \
  GetNextLine/GetNextLine \
  Libft/LibFT \
  Logger/Logger \
  Printf/Printf \
  ReadLine/ReadLine \
  PThread/PThread \
  CPP_class/CPP_class \
  Errno/errno \
  Config/config \
  Networking/networking \
  API/API \
  encryption/encryption \
  RNG/RNG \
  JSon/JSon \
  file/file \
  HTML/HTMLParser \
  Game/Game

ifeq ($(OS),Windows_NT)
LIB_BASES += Windows/Windows
else
LIB_BASES += Linux/Linux
endif

LIBS       := $(addsuffix .a, $(LIB_BASES))
DEBUG_LIBS := $(addsuffix _debug.a, $(LIB_BASES))

TARGET        := Full_Libft.a
DEBUG_TARGET  := Full_Libft_debug.a

all: $(TARGET)

debug: $(DEBUG_TARGET)

both: all debug

re: fclean all

define EXTRACT
cd temp_objs && $(AR) x ../$1 && cd ..;
endef

$(TARGET): $(LIBS)
	@echo "Linking libraries into $(TARGET)..."
	$(RM) $@
	$(MKDIR) temp_objs
	$(foreach lib,$(LIBS),$(call EXTRACT,$(lib)))
	$(AR) $(ARFLAGS) $@ temp_objs/*.o
	$(RMDIR) temp_objs

$(DEBUG_TARGET): $(DEBUG_LIBS)
	@echo "Linking libraries into $(DEBUG_TARGET)..."
	$(RM) $@
	$(MKDIR) temp_objs
	$(foreach lib,$(DEBUG_LIBS),$(call EXTRACT,$(lib)))
	$(AR) $(ARFLAGS) $@ temp_objs/*.o
	$(RMDIR) temp_objs

%.a:
	$(MAKE) -C $(dir $@)

%_debug.a:
	$(MAKE) -C $(dir $@) debug

clean:
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) clean;)
	$(RM) $(TARGET) $(DEBUG_TARGET)

fclean:
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) fclean;)
	$(RM) $(TARGET) $(DEBUG_TARGET)
    
.PHONY: all debug both re clean fclean
