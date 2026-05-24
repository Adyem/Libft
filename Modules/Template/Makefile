include ../../mk/compiler_flags.mk
include ../../mk/modules/Template.mk

all: $(TARGET)
	@if $(RM) $(CLEAN_FILES); then \
        printf '        [%s] template verification passed (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS); \
    else \
        printf '        [%s] cleanup failed\n' "$(MODULE_NAME)"; \
        exit 1; \
    fi

$(TARGET): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@printf '        [%s] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR):
	@$(MKDIR) $@

clean:
	@if $(RM) $(CLEAN_FILES); then \
        printf '        [%s] clean: success\n' "$(MODULE_NAME)"; \
    else \
        printf '        [%s] clean: failed\n' "$(MODULE_NAME)"; \
        exit 1; \
    fi

fclean: clean

re:
	@$(MAKE) fclean
	@$(MAKE) all


%_test.a:
	@$(MAKE) TARGET="$@" $(SUBMAKE_OVERRIDES) COMPILE_FLAGS="$(COMPILE_FLAGS) -DLIBFT_TEST_BUILD" all

-include $(DEPS) $(DEBUG_DEPS)

.PHONY: all clean fclean re
