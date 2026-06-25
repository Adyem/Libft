CLEAN_OBJS := $(shell find . -type f \( -name *.o -o -name *.d \
	-o -name *.gcda -o -name *.gcno \) 2>/dev/null)
CLEAN_DIRS := $(wildcard objs*)

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
    CLEAN_DIRS := $(subst /,\\,$(CLEAN_DIRS))
else
    CLEAN_FILES := $(CLEAN_OBJS)
endif
