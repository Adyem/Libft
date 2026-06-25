#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Observability/observability.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"

static ft_observability_trace_event g_file_trace_last_event;
static uint32_t g_file_trace_count = 0;

static void test_observability_file_trace_hook(const ft_observability_trace_event &event)
{
    g_file_trace_last_event = event;
    g_file_trace_count = g_file_trace_count + 1;
    return ;
}

FT_TEST(test_observability_file_counters_and_trace_hook)
{
    ft_observability_counters counters;
    ft_string content;
    const char *path;

    path = "test_observability_file_counters.txt";
    (void)file_delete(path);
    g_file_trace_count = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        observability_reset_counters(FT_OBSERVABILITY_MODULE_FILE));
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        observability_set_trace_hook(&test_observability_file_trace_hook));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(path, "abc", 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, content.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_read_all(path, content));

    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        observability_get_counters(FT_OBSERVABILITY_MODULE_FILE, &counters));
    FT_ASSERT(counters.operations >= 2);
    FT_ASSERT(counters.successes >= 2);
    FT_ASSERT_EQ(0, counters.failures);
    FT_ASSERT(counters.bytes_read >= 3);
    FT_ASSERT(counters.bytes_written >= 3);
    FT_ASSERT(g_file_trace_count >= 4);
    FT_ASSERT_EQ(FT_OBSERVABILITY_MODULE_FILE, g_file_trace_last_event.module);
    FT_ASSERT_EQ(FT_OBSERVABILITY_TRACE_FINISH, g_file_trace_last_event.phase);

    observability_clear_trace_hook();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, content.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_delete(path));
    return (1);
}
