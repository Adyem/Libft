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
            Printf \
            ReadLine \
            PThread \
            CPP_class \
            Errno \
            Networking

ifeq ($(OS),Windows_NT)
SUBDIRS += Windows
else
SUBDIRS += Linux
endif

SUBDIRS += encryption \
            RNG \
            JSon \
            file \
            HTML \
            Game

LIB_BASES := \
  CMA/CustomMemoryAllocator \
  GetNextLine/GetNextLine \
  Libft/LibFT \
  Printf/Printf \
  ReadLine/ReadLine \
  PThread/PThread \
  CPP_class/CPP_class \
  Errno/errno \
  Networking/networking

ifeq ($(OS),Windows_NT)
LIB_BASES += Windows/Windows
else
LIB_BASES += Linux/Linux
endif

LIB_BASES += encryption/encryption \
  RNG/RNG \
  JSon/JSon \
  file/file \
  HTML/HTMLParser \
  Game/Game

ifeq ($(OS),Windows_NT)
OS_EXTRACT = $(call EXTRACT,Windows/Windows.a)
DEBUG_OS_EXTRACT = $(call EXTRACT,Windows/Windows_debug.a)
OS_CLEAN := $(MAKE) -C Windows clean
OS_FCLEAN := $(MAKE) -C Windows fclean
else
OS_EXTRACT = $(call EXTRACT,Linux/Linux.a)
DEBUG_OS_EXTRACT = $(call EXTRACT,Linux/Linux_debug.a)
OS_CLEAN := $(MAKE) -C Linux clean
OS_FCLEAN := $(MAKE) -C Linux fclean
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
cd temp_objs && $(AR) x ../$1 && cd ..
endef

$(TARGET): $(LIBS)
	@echo "Linking libraries into $(TARGET)..."
	$(RM) $@
	$(MKDIR) temp_objs
	$(call EXTRACT,CMA/CustomMemoryAllocator.a)
	$(call EXTRACT,GetNextLine/GetNextLine.a)
	$(call EXTRACT,Libft/LibFT.a)
	$(call EXTRACT,Printf/Printf.a)
	$(call EXTRACT,ReadLine/ReadLine.a)
	$(call EXTRACT,PThread/PThread.a)
	$(call EXTRACT,CPP_class/CPP_class.a)
	$(call EXTRACT,Errno/errno.a)
	$(call EXTRACT,Networking/networking.a)
	$(OS_EXTRACT)
	$(call EXTRACT,encryption/encryption.a)
	$(call EXTRACT,RNG/RNG.a)
	$(call EXTRACT,JSon/JSon.a)
	$(call EXTRACT,file/file.a)
	$(call EXTRACT,HTML/HTMLParser.a)
	$(call EXTRACT,Game/Game.a)
	$(AR) $(ARFLAGS) $@ temp_objs/*.o
	$(RMDIR) temp_objs

$(DEBUG_TARGET): $(DEBUG_LIBS)
	@echo "Linking libraries into $(DEBUG_TARGET)..."
	$(RM) $@
	$(MKDIR) temp_objs
	$(call EXTRACT,CMA/CustomMemoryAllocator_debug.a)
	$(call EXTRACT,GetNextLine/GetNextLine_debug.a)
	$(call EXTRACT,Libft/LibFT_debug.a)
	$(call EXTRACT,Printf/Printf_debug.a)
	$(call EXTRACT,ReadLine/ReadLine_debug.a)
	$(call EXTRACT,PThread/PThread_debug.a)
	$(call EXTRACT,CPP_class/CPP_class_debug.a)
	$(call EXTRACT,Errno/errno_debug.a)
	$(call EXTRACT,Networking/networking_debug.a)
	$(DEBUG_OS_EXTRACT)
	$(call EXTRACT,encryption/encryption_debug.a)
	$(call EXTRACT,RNG/RNG_debug.a)
	$(call EXTRACT,JSon/JSon_debug.a)
	$(call EXTRACT,file/file_debug.a)
	$(call EXTRACT,HTML/HTMLParser_debug.a)
	$(call EXTRACT,Game/Game_debug.a)
	$(AR) $(ARFLAGS) $@ temp_objs/*.o
	$(RMDIR) temp_objs

%.a:
	$(MAKE) -C $(dir $@)

%_debug.a:
	$(MAKE) -C $(dir $@) debug

clean:
	$(MAKE) -C CMA clean
	$(MAKE) -C GetNextLine clean
	$(MAKE) -C Libft clean
	$(MAKE) -C Printf clean
	$(MAKE) -C ReadLine clean
	$(MAKE) -C PThread clean
	$(MAKE) -C CPP_class clean
	$(MAKE) -C Errno clean
	$(MAKE) -C Networking clean
	$(OS_CLEAN)
	$(MAKE) -C encryption clean
	$(MAKE) -C RNG clean
	$(MAKE) -C JSon clean
	$(MAKE) -C file clean
	$(MAKE) -C HTML clean
	$(MAKE) -C Game clean
	$(RM) $(TARGET) $(DEBUG_TARGET)

fclean: clean
	$(MAKE) -C CMA fclean
	$(MAKE) -C GetNextLine fclean
	$(MAKE) -C Libft fclean
	$(MAKE) -C Printf fclean
	$(MAKE) -C ReadLine fclean
	$(MAKE) -C PThread fclean
	$(MAKE) -C CPP_class fclean
	$(MAKE) -C Errno fclean
	$(MAKE) -C Networking fclean
	$(OS_FCLEAN)
	$(MAKE) -C encryption fclean
	$(MAKE) -C RNG fclean
	$(MAKE) -C JSon fclean
	$(MAKE) -C file fclean
	$(MAKE) -C HTML fclean
	$(MAKE) -C Game fclean
	$(RM) $(TARGET) $(DEBUG_TARGET)
	
.PHONY: all debug both re clean fclean
