#include "../test_internal.hpp"
#include "../../Modules/Sink/sink.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_sink_record_message_and_retrieve)
{
    sink_record record;
    ft_size_t record_count;

    sink_clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_record_message(12, "first message"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_record_message(34, "second message"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record_count(&record_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), record_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record(0, &record));
    FT_ASSERT_EQ(12, record.level);
    FT_ASSERT_EQ(0, ft_strcmp(record.message, "first message"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record(1, &record));
    FT_ASSERT_EQ(34, record.level);
    FT_ASSERT_EQ(0, ft_strcmp(record.message, "second message"));
    sink_clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record_count(&record_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), record_count);
    return (1);
}

FT_TEST(test_sink_rejects_null_message)
{
    sink_clear();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, sink_record_message(1, ft_nullptr));
    return (1);
}

FT_TEST(test_sink_reports_invalid_arguments_and_out_of_range)
{
    sink_record record;
    ft_size_t record_count;

    sink_clear();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, sink_get_record_count(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, sink_get_record(0, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, sink_get_record(0, &record));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_record_message(7, "validation message"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record_count(&record_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(1), record_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record(0, &record));
    FT_ASSERT_EQ(7, record.level);
    FT_ASSERT_EQ(0, ft_strcmp(record.message, "validation message"));
    sink_clear();
    return (1);
}

FT_TEST(test_sink_clear_is_idempotent_and_reusable)
{
    sink_record record;
    ft_size_t record_count;

    sink_clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record_count(&record_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), record_count);
    sink_clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record_count(&record_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), record_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_record_message(22, "first reuse message"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record_count(&record_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(1), record_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record(0, &record));
    FT_ASSERT_EQ(22, record.level);
    FT_ASSERT_EQ(0, ft_strcmp(record.message, "first reuse message"));
    sink_clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record_count(&record_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), record_count);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, sink_get_record(0, &record));
    return (1);
}
