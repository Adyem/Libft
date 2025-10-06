#include "pthread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include <climits>
#include <unistd.h>

static void pt_mutex_report_wake_failure(int wake_error)
{
    char message_buffer[128];
    const char *prefix_string;
    size_t prefix_index;
    int absolute_error;
    char digit_buffer[16];
    size_t digit_index;
    size_t message_index;

    prefix_string = "pt_mutex::unlock wake failure: ";
    prefix_index = 0;
    message_index = 0;
    while (prefix_string[prefix_index] != '\0' && message_index < sizeof(message_buffer) - 1)
    {
        message_buffer[message_index] = prefix_string[prefix_index];
        prefix_index += 1;
        message_index += 1;
    }
    if (wake_error < 0 && message_index < sizeof(message_buffer) - 1)
    {
        message_buffer[message_index] = '-';
        message_index += 1;
        if (wake_error == INT_MIN)
            wake_error = INT_MAX;
        else
            wake_error = -wake_error;
    }
    absolute_error = wake_error;
    digit_index = 0;
    if (absolute_error == 0)
    {
        digit_buffer[digit_index] = '0';
        digit_index += 1;
    }
    else
    {
        while (absolute_error != 0 && digit_index < sizeof(digit_buffer))
        {
            digit_buffer[digit_index] = static_cast<char>('0' + (absolute_error % 10));
            absolute_error = absolute_error / 10;
            digit_index += 1;
        }
    }
    while (digit_index > 0 && message_index < sizeof(message_buffer) - 1)
    {
        digit_index -= 1;
        message_buffer[message_index] = digit_buffer[digit_index];
        message_index += 1;
    }
    if (message_index < sizeof(message_buffer) - 1)
    {
        message_buffer[message_index] = '\n';
        message_index += 1;
    }
    write(2, message_buffer, message_index);
    return ;
}


int pt_mutex::unlock(pthread_t thread_id)
{
    this->set_error(ER_SUCCESS);
    if (this->_owner.load(std::memory_order_relaxed) != thread_id)
    {
        ft_errno = PT_ERR_MUTEX_OWNER;
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (-1);
    }
    this->_owner.store(0, std::memory_order_relaxed);
    this->_lock = false;
    this->_serving.fetch_add(1, std::memory_order_release);
    int wake_result;
    int wake_error;
    int wake_attempts;

    wake_error = ER_SUCCESS;
    wake_attempts = 0;
    while (1)
    {
        wake_result = pt_thread_wake_one_uint32(&this->_serving);
        if (wake_result == 0)
            break;
        wake_error = ft_errno;
        if (wake_attempts >= 4)
            break;
        wake_attempts += 1;
    }
    if (wake_result != 0)
    {
        ft_errno = wake_error;
        this->set_error(wake_error);
        pt_mutex_report_wake_failure(wake_error);
        return (-1);
    }
    return (FT_SUCCESS);
}

