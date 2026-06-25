#include "../test_internal.hpp"
#include "../../Modules/Logger/logger.hpp"
#include "../../Modules/Sink/sink.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"

static void logger_enqueue_message(const char *format_string, ...)
{
    va_list argument_list;

    va_start(argument_list, format_string);
    ft_log_enqueue(LOG_LEVEL_INFO, format_string, argument_list);
    va_end(argument_list);
    return ;
}

FT_TEST(test_logger_info_records_sink_entry)
{
    int pipe_descriptors[2];
    int write_descriptor;
    ssize_t read_count;
    char buffer[512];
    sink_record record;
    ft_size_t record_count;
    ft_bool previous_color;

    sink_clear();
    ft_log_enable_async(false);
    ft_log_clear_redactions();
    previous_color = ft_log_get_color();
    ft_log_set_color(false);
    FT_ASSERT_EQ(0, pipe(pipe_descriptors));
    write_descriptor = pipe_descriptors[1];
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_json_sink, &write_descriptor));
    ft_log_info("logger sink plain");
    read_count = read(pipe_descriptors[0], buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(ft_strstr(buffer, "\"message\":\"logger sink plain\"") != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record_count(&record_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(1), record_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record(0, &record));
    FT_ASSERT_EQ(LOG_LEVEL_INFO, record.level);
    FT_ASSERT(ft_strstr(record.message, "level=INFO") != ft_nullptr);
    FT_ASSERT(ft_strstr(record.message, "severity=20") != ft_nullptr);
    FT_ASSERT(ft_strstr(record.message, "message=\"logger sink plain\"") != ft_nullptr);
    ft_log_remove_sink(ft_json_sink, &write_descriptor);
    ft_log_set_color(previous_color);
    ft_log_clear_redactions();
    sink_clear();
    close(pipe_descriptors[0]);
    close(pipe_descriptors[1]);
    return (1);
}

FT_TEST(test_logger_enqueue_records_sink_entries_in_order)
{
    int pipe_descriptors[2];
    int write_descriptor;
    ssize_t read_count;
    char buffer[1024];
    sink_record record;
    ft_size_t record_count;

    sink_clear();
    ft_log_clear_redactions();
    ft_log_enable_async(false);
    FT_ASSERT_EQ(0, pipe(pipe_descriptors));
    write_descriptor = pipe_descriptors[1];
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_json_sink, &write_descriptor));
    ft_log_enable_async(true);
    logger_enqueue_message("logger sink queue %d", 42);
    logger_enqueue_message("logger sink queue %d", 43);
    ft_log_enable_async(false);
    read_count = read(pipe_descriptors[0], buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(ft_strstr(buffer, "\"message\":\"logger sink queue 42\"") != ft_nullptr);
    FT_ASSERT(ft_strstr(buffer, "\"message\":\"logger sink queue 43\"") != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record_count(&record_count));
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), record_count);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record(0, &record));
    FT_ASSERT_EQ(LOG_LEVEL_INFO, record.level);
    FT_ASSERT(ft_strstr(record.message, "logger sink queue 42") != ft_nullptr);
    FT_ASSERT(ft_strstr(record.message, "level=INFO") != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sink_get_record(1, &record));
    FT_ASSERT_EQ(LOG_LEVEL_INFO, record.level);
    FT_ASSERT(ft_strstr(record.message, "logger sink queue 43") != ft_nullptr);
    FT_ASSERT(ft_strstr(record.message, "severity=20") != ft_nullptr);
    ft_log_remove_sink(ft_json_sink, &write_descriptor);
    ft_log_clear_redactions();
    sink_clear();
    close(pipe_descriptors[0]);
    close(pipe_descriptors[1]);
    return (1);
}
