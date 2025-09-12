TARGET := Math.a
DEBUG_TARGET := Math_debug.a

SRCS := math_abs.cpp \
        math_fabs.cpp \
        math_signbit.cpp \
        math_isnan.cpp \
        math_nan.cpp \
        math_swap.cpp \
        math_clamp.cpp \
        math_gcd.cpp \
        math_lcm.cpp \
        math_max.cpp \
        math_min.cpp \
        math_factorial.cpp \
        math_absdiff.cpp \
        math_average.cpp \
        math_pow.cpp \
        math_sqrt.cpp \
        math_exp.cpp \
        math_log.cpp \
        math_cos.cpp \
        math_sin.cpp \
        math_tan.cpp \
        math_deg2rad.cpp \
        math_rad2deg.cpp \
        math_roll.cpp \
        math_eval.cpp \
        math_roll_utilities.cpp \
        math_roll_parse_brackets.cpp \
        math_roll_parse_dice.cpp \
        math_roll_parse_md.cpp \
        math_roll_parse_pm.cpp \
        math_roll_parse_utils.cpp \
        math_roll_validate_string.cpp \
        math_roll_validate_utils.cpp \
        math_validate_int.cpp \
        linear_algebra.cpp

HEADERS := math.hpp roll.hpp math_internal.hpp linear_algebra.hpp

ifeq ($(OS),Windows_NT)
    MKDIR   = mkdir
    RM      = del /F /Q
else
    MKDIR   = mkdir -p
    RM      = rm -f
endif

ifdef COMPILE_FLAGS
    CFLAGS := $(COMPILE_FLAGS)
endif

CXX       := g++
AR        := ar
ARFLAGS   := rcs

OBJDIR         := objs
DEBUG_OBJDIR   := objs_debug

OBJS       := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUG_OBJDIR)/%.o,$(SRCS))

CFLAGS   ?= -Wall -Wextra -Werror -g -O0 -std=c++17

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	$(CXX) $(CFLAGS) -c $< -o $@

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
	$(CXX) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(DEBUG_OBJDIR):
	$(MKDIR) $@

CLEAN_OBJS := $(wildcard $(OBJDIR)/*.o) $(wildcard $(DEBUG_OBJDIR)/*.o)

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
else
    CLEAN_FILES := $(CLEAN_OBJS)
endif

clean:
	-$(RM) $(CLEAN_FILES)

fclean: clean
	-$(RM) $(TARGET) $(DEBUG_TARGET)

re: fclean all

both: all debug

.PHONY: all clean fclean re debug both

