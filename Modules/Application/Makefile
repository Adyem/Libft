include ../../mk/compiler_flags.mk
include ../../mk/modules/common/module_defaults.mk

TARGET := Application.a
DEBUG_TARGET := Application_debug.a

SRCS := application_auth_service.cpp

HEADERS := application.hpp application_auth_service.hpp

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@current=0; \
	for src in $(SRCS) $(MM_SRCS); do \
		current=$$((current + 1)); \
		printf '        [%s] Built file %s (%d/%d)\n' "$(MODULE_NAME)" "$$src" $$current $(TOTAL_SRCS); \
	done
	@printf '        [%s] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(OBJDIR)/%.o: %.cpp $(HEADERS) | $(OBJDIR)
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@

debug: CXXFLAGS += -DDEBUG=1
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	@$(AR) $(ARFLAGS) $@ $^
	@current=0; \
	for src in $(SRCS) $(MM_SRCS); do \
		current=$$((current + 1)); \
		printf '        [%s Debug] Built file %s (%d/%d)\n' "$(MODULE_NAME)" "$$src" $$current $(TOTAL_SRCS); \
	done
	@printf '        [%s Debug] Module archive ready (%d files)\n' "$(MODULE_NAME)" $(TOTAL_SRCS)

$(DEBUG_OBJDIR)/%.o: %.cpp $(HEADERS) | $(DEBUG_OBJDIR)
	@$(CXX) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJDIR) $(DEBUG_OBJDIR):
	@$(MKDIR) $@

%_test.a: FORCE
	@$(MAKE) TARGET="$@" $(SUBMAKE_OVERRIDES) COMPILE_FLAGS="$(COMPILE_FLAGS) -DLIBFT_TEST_BUILD" all

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

fclean:
	@status=0; \
	if ! $(RM) $(FCLEAN_FILES); then \
		status=1; \
	fi; \
	if ! $(RMDIR) $(CLEAN_DIRS); then \
		status=1; \
	fi; \
	if [ $$status -eq 0 ]; then \
		printf '        [%s] fclean: success\n' "$(MODULE_NAME)"; \
	else \
		printf '        [%s] fclean: failed\n' "$(MODULE_NAME)"; \
		exit 1; \
	fi

re:
	@$(MAKE) fclean
	@$(MAKE) all

both: all debug

-include $(DEPS) $(DEBUG_DEPS)

.PHONY: all clean fclean re debug both
