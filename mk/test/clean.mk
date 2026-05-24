CLEAN_OBJS := $(shell find . -type f \( -name '*.o' -o -name '*.d' \) 2>/dev/null)

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
else
    CLEAN_FILES := $(CLEAN_OBJS)
endif
