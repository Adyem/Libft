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
            Networking \
            Windows \
            Linux \
            encryption \
            RNG \
                        JSon \
                        file \
            HTML

LIB_BASES := \
  CMA/CustomMemoryAllocator \
  GetNextLine/GetNextLine \
  Libft/LibFT \
  Printf/Printf \
  ReadLine/ReadLine \
  PThread/PThread \
  CPP_class/CPP_class \
  Errno/errno \
  Networking/networking \
  Windows/Windows \
  Linux/Linux \
  encryption/encryption \
  RNG/RNG \
  JSon/JSon \
  file/file \
  HTML/HTMLParser

LIBS       := $(addsuffix .a, $(LIB_BASES))
DEBUG_LIBS := $(addsuffix _debug.a, $(LIB_BASES))

TARGET        := Full_Libft.a
DEBUG_TARGET  := Full_Libft_debug.a

all: $(TARGET)

debug: $(DEBUG_TARGET)

both: all debug

re: fclean all

$(TARGET): $(LIBS)
	@echo "Linking libraries into $(TARGET)..."
	$(RM) $@
	$(MKDIR) temp_objs
	@for lib in $(LIBS); do \
	  cd temp_objs && $(AR) x ../$$lib; \
	  cd ..; \
	done
	$(AR) $(ARFLAGS) $@ temp_objs/*.o
	$(RMDIR) temp_objs

$(DEBUG_TARGET): $(DEBUG_LIBS)
	@echo "Linking libraries into $(DEBUG_TARGET)..."
	$(RM) $@
	$(MKDIR) temp_objs
	@for lib in $(DEBUG_LIBS); do \
	  cd temp_objs && $(AR) x ../$$lib; \
	  cd ..; \
	done
	$(AR) $(ARFLAGS) $@ temp_objs/*.o
	$(RMDIR) temp_objs

%.a:
	$(MAKE) -C $(dir $@)

%_debug.a:
	$(MAKE) -C $(dir $@) debug

clean:
	@for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir clean; \
	done
	$(RM) $(TARGET) $(DEBUG_TARGET)

fclean: clean
	@for dir in $(SUBDIRS); do \
	  $(MAKE) -C $$dir fclean; \
	done
	$(RM) $(TARGET) $(DEBUG_TARGET)

.PHONY: all debug both re clean fclean
