#include "../../Logger/logger.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include <pthread.h>

struct s_sink_test_data
{
    pthread_mutex_t mutex;
    size_t          call_count;
    bool            encountered_error;
};

struct s_thread_arguments
{
    size_t              iterations;
    s_sink_test_data   *sink_data;
};

static void ft_record_sink_error(s_sink_test_data *data)
{
    if (!data)
        return ;
    if (pthread_mutex_lock(&data->mutex) != 0)
        return ;
    data->encountered_error = true;
    pthread_mutex_unlock(&data->mutex);
    return ;
}

static void ft_mutex_test_sink(const char *message, void *user_data)
{
    s_sink_test_data *data;

    (void)message;
    data = static_cast<s_sink_test_data *>(user_data);
    if (!data)
        return ;
    if (pthread_mutex_lock(&data->mutex) != 0)
    {
        ft_record_sink_error(data);
        return ;
    }
    data->call_count++;
    if (pthread_mutex_unlock(&data->mutex) != 0)
        ft_record_sink_error(data);
    return ;
}

static void *ft_logger_sink_toggle_thread(void *argument)
{
    s_thread_arguments *arguments;
    size_t iteration;

    arguments = static_cast<s_thread_arguments *>(argument);
    iteration = 0;
    while (iteration < arguments->iterations)
    {
        int add_result;

        add_result = ft_log_add_sink(ft_mutex_test_sink, arguments->sink_data);
        if (add_result == 0)
        {
            ft_log_info("threaded sink message");
            ft_log_remove_sink(ft_mutex_test_sink, arguments->sink_data);
            if (ft_errno != ER_SUCCESS)
                ft_record_sink_error(arguments->sink_data);
        }
        else
            ft_record_sink_error(arguments->sink_data);
        iteration++;
    }
    return (ft_nullptr);
}

FT_TEST(test_logger_sink_mutex_thread_safety, "logger protects sinks during concurrent updates")
{
    const size_t thread_count = 4;
    const size_t iterations = 64;
    pthread_t threads[thread_count];
    s_sink_test_data sink_data = {PTHREAD_MUTEX_INITIALIZER, 0, false};
    s_thread_arguments arguments;
    size_t index;
    size_t log_iteration;

    ft_log_close();
    arguments.iterations = iterations;
    arguments.sink_data = &sink_data;
    index = 0;
    while (index < thread_count)
    {
        FT_ASSERT_EQ(0, pthread_create(&threads[index], ft_nullptr, ft_logger_sink_toggle_thread, &arguments));
        index++;
    }
    log_iteration = 0;
    while (log_iteration < iterations)
    {
        ft_log_debug("main thread logger message");
        log_iteration++;
    }
    index = 0;
    while (index < thread_count)
    {
        FT_ASSERT_EQ(0, pthread_join(threads[index], ft_nullptr));
        index++;
    }
    size_t recorded_call_count;
    bool encountered_error;

    if (pthread_mutex_lock(&sink_data.mutex) != 0)
    {
        ft_log_close();
        FT_ASSERT(false);
        return (1);
    }
    recorded_call_count = sink_data.call_count;
    encountered_error = sink_data.encountered_error;
    pthread_mutex_unlock(&sink_data.mutex);
    ft_log_close();
    FT_ASSERT(!encountered_error);
    FT_ASSERT_EQ(thread_count * iterations, recorded_call_count);
    return (1);
}
