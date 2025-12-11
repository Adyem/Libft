#include "../../Logger/logger.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"

struct logger_async_sink_context
{
    size_t handled_messages;
};

static void logger_async_sink(const char *message, void *user_data)
{
    logger_async_sink_context *context;

    (void)message;
    context = static_cast<logger_async_sink_context *>(user_data);
    if (context == ft_nullptr)
        return ;
    context->handled_messages = context->handled_messages + 1;
    return ;
}

FT_TEST(test_logger_async_logging_teardown, "logger async logging")
{
    logger_async_sink_context context;
    size_t original_limit;
    size_t message_index;
    int sink_registered;
    int success;

    context.handled_messages = 0;
    sink_registered = 0;
    success = 1;
    ft_log_enable_async(false);
    if (ft_errno != FT_ERR_SUCCESSS)
        success = 0;
    original_limit = ft_log_get_async_queue_limit();
    if (ft_errno != FT_ERR_SUCCESSS)
        success = 0;
    if (success)
    {
        ft_log_set_async_queue_limit(0);
        if (ft_errno != FT_ERR_SUCCESSS)
            success = 0;
    }
    if (success)
    {
        if (ft_log_add_sink(logger_async_sink, &context) != 0)
            success = 0;
        else
            sink_registered = 1;
    }
    if (success)
    {
        ft_log_reset_async_metrics();
        if (ft_errno != FT_ERR_SUCCESSS)
            success = 0;
    }
    if (success)
    {
        ft_log_enable_async(true);
        if (ft_errno != FT_ERR_SUCCESSS)
            success = 0;
    }
    if (success)
    {
        message_index = 0;
        while (message_index < 64)
        {
            ft_log_info("async regression message %u", static_cast<unsigned int>(message_index));
            message_index++;
        }
    }
    ft_log_enable_async(false);
    if (ft_errno != FT_ERR_SUCCESSS)
        success = 0;
    if (sink_registered)
        ft_log_remove_sink(logger_async_sink, &context);
    ft_log_set_async_queue_limit(original_limit);
    if (ft_errno != FT_ERR_SUCCESSS)
        success = 0;
    if (!success)
        return (0);
    FT_ASSERT(context.handled_messages >= 64);
    return (1);
}
