include ../compiler_flags.mk
TARGET := Math.a
DEBUG_TARGET := Math_debug.a

SRCS := math_abs.cpp \
        math_fabs.cpp \
        math_fmod.cpp \
        math_signbit.cpp \
        math_isnan.cpp \
        math_isinf.cpp \
        math_nan.cpp \
        math_infinity.cpp \
        math_negative_infinity.cpp \
        math_indeterminate.cpp \
        math_swap.cpp \
        math_clamp.cpp \
        math_gcd.cpp \
        math_big_number.cpp \
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
        math_acos.cpp \
        math_cos.cpp \
        math_sin.cpp \
        math_tan.cpp \
        math_deg2rad.cpp \
        math_rad2deg.cpp \
        math_statistics.cpp \
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
        math_autodiff.cpp \
        math_fft.cpp \
        math_interval.cpp \
        math_polynomial.cpp \
        linear_algebra.cpp \
        linear_algebra_constructors.cpp \
        linear_algebra_vector2.cpp \
        linear_algebra_vector3.cpp \
        linear_algebra_vector4.cpp \
        linear_algebra_quaternion.cpp

MODULE_NAME := $(notdir $(CURDIR))
TOTAL_SRCS := $(words $(SRCS))

HEADERS := math.hpp roll.hpp math_internal.hpp linear_algebra.hpp linear_algebra_quaternion.hpp math_autodiff.hpp math_fft.hpp math_interval.hpp math_polynomial.hpp

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

BUILD_OBJ_SUFFIX :=
ifneq ($(findstring -DLIBFT_TEST_BUILD,$(COMPILE_FLAGS)),)
    BUILD_OBJ_SUFFIX := _test
endif

OBJDIR         := objs$(BUILD_OBJ_SUFFIX)
DEBUG_OBJDIR   := objs_debug$(BUILD_OBJ_SUFFIX)

OBJS       := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEBUG_OBJS := $(patsubst %.cpp,$(DEBUG_OBJDIR)/%.o,$(SRCS))
DEPS       := $(OBJS:.o=.d)
DEBUG_DEPS := $(DEBUG_OBJS:.o=.d)

CFLAGS   ?= -Wall -Wextra -Werror -g -O0 -std=c++17 -Wuseless-cast

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@
	@count=$$(find $(OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
	    printf '        [%s] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s Debug] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@
	@count=$$(find $(DEBUG_OBJDIR) -maxdepth 1 -name '*.o' 2>/dev/null | wc -l); \
	    printf '        [%s Debug] Building file %s (%d/%d)\n' "$(MODULE_NAME)" "$<" $$count $(TOTAL_SRCS)

$(OBJDIR) $(DEBUG_OBJDIR):
	@$(MKDIR) $@

CLEAN_OBJS := $(wildcard $(OBJDIR)/*.o) $(wildcard $(OBJDIR)/*.d) $(wildcard $(DEBUG_OBJDIR)/*.o) $(wildcard $(DEBUG_OBJDIR)/*.d)

ifeq ($(OS),Windows_NT)
    CLEAN_FILES := $(subst /,\\,$(CLEAN_OBJS))
else
    CLEAN_FILES := $(CLEAN_OBJS)
endif

clean:
	@if $(RM) $(CLEAN_FILES); then \
		printf '        [%s] clean: success\n' "$(MODULE_NAME)"; \
	else \
		printf '        [%s] clean: failed\n' "$(MODULE_NAME)"; \
		exit 1; \
	fi

fclean:
	@status=0; \
	if ! $(RM) $(CLEAN_FILES); then \
		status=1; \
	fi; \
	if ! $(RM) $(TARGET) $(DEBUG_TARGET); then \
		status=1; \
	fi; \
	if [ $$status -eq 0 ]; then \
		printf '        [%s] fclean: success\n' "$(MODULE_NAME)"; \
	else \
		printf '        [%s] fclean: failed\n' "$(MODULE_NAME)"; \
		exit 1; \
	fi

re: fclean all

both: all debug

-include $(DEPS) $(DEBUG_DEPS)

.PHONY: all clean fclean re debug both


