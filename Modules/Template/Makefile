include ../../mk/compiler_flags.mk
include ../../mk/modules/Template.mk

all: $(TARGET)
	@status=0; \
	if ! $(RM) $(CLEAN_FILES); then \
		status=1; \
	fi; \
	if ! $(RMDIR) $(CLEAN_DIRS); then \
		status=1; \
	fi; \
	if [ $$status -eq 0 ]; then \
		printf '        [%s] template verification passed (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS); \
	else \
		printf '        [%s] cleanup failed\n' "$(MODULE_NAME)"; \
		exit 1; \
	fi

$(TARGET): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@current=0; \
	for src in $(SRCS) $(MM_SRCS); do \
		current=$$((current + 1)); \
		printf '        [%s] Built file %s (%d/%d)\n' "$(MODULE_NAME)" "$$src" $$current $(TOTAL_SRCS); \
	done
	@printf '        [%s] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR):
	@$(MKDIR) $@

clean:
	@status=0; \
	if ! $(RM) $(CLEAN_FILES); then \
		status=1; \
	fi; \
	if ! $(RMDIR) $(CLEAN_DIRS); then \
		status=1; \
	fi; \
	if [ $$status -eq 0 ]; then \
		printf '        [%s] clean: success\n' "$(MODULE_NAME)"; \
	else \
		printf '        [%s] clean: failed\n' "$(MODULE_NAME)"; \
		exit 1; \
	fi
fclean: clean

re:
	@$(MAKE) fclean
	@$(MAKE) all


%_test.a: FORCE
	@$(MAKE) TARGET="$@" $(SUBMAKE_OVERRIDES) COMPILE_FLAGS="$(COMPILE_FLAGS) -DLIBFT_TEST_BUILD" all

-include $(DEPS) $(DEBUG_DEPS)

.PHONY: all clean fclean re
