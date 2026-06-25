#include "sink.hpp"
#include <atomic>
#include <cstdlib>

struct sink_node
{
    sink_record record;
    char *message_storage;
    sink_node *next;
};

static std::atomic_flag g_sink_lock = ATOMIC_FLAG_INIT;
static sink_node *g_sink_head = ft_nullptr;
static sink_node *g_sink_tail = ft_nullptr;
static ft_size_t g_sink_count = 0;

static void sink_lock(void)
{
    while (g_sink_lock.test_and_set(std::memory_order_acquire))
    {
    }
    return ;
}

static void sink_unlock(void)
{
    g_sink_lock.clear(std::memory_order_release);
    return ;
}

static char *sink_duplicate_message(const char *message)
{
    char *copy;
    ft_size_t length;

    if (message == ft_nullptr)
        return (ft_nullptr);
    length = ft_strlen_size_t(message);
    copy = static_cast<char *>(std::malloc(length + 1));
    if (copy == ft_nullptr)
        return (ft_nullptr);
    ft_memcpy(copy, message, length + 1);
    return (copy);
}

int32_t sink_record_message(int32_t level, const char *message) noexcept
{
    sink_node *node;
    char *message_copy;

    if (message == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    message_copy = sink_duplicate_message(message);
    if (message_copy == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    node = static_cast<sink_node *>(std::malloc(sizeof(sink_node)));
    if (node == ft_nullptr)
    {
        std::free(message_copy);
        return (FT_ERR_NO_MEMORY);
    }
    node->record.level = level;
    node->record.message = message_copy;
    node->message_storage = message_copy;
    node->next = ft_nullptr;
    sink_lock();
    if (g_sink_tail == ft_nullptr)
        g_sink_head = node;
    else
        g_sink_tail->next = node;
    g_sink_tail = node;
    g_sink_count += 1;
    sink_unlock();
    return (FT_ERR_SUCCESS);
}

int32_t sink_get_record_count(ft_size_t *count_out) noexcept
{
    if (count_out == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    sink_lock();
    *count_out = g_sink_count;
    sink_unlock();
    return (FT_ERR_SUCCESS);
}

int32_t sink_get_record(ft_size_t index, sink_record *record_out) noexcept
{
    sink_node *current_node;
    ft_size_t current_index;

    if (record_out == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    sink_lock();
    if (index >= g_sink_count)
    {
        sink_unlock();
        return (FT_ERR_OUT_OF_RANGE);
    }
    current_node = g_sink_head;
    current_index = 0;
    while (current_node != ft_nullptr && current_index < index)
    {
        current_node = current_node->next;
        current_index += 1;
    }
    if (current_node == ft_nullptr)
    {
        sink_unlock();
        return (FT_ERR_INVALID_STATE);
    }
    *record_out = current_node->record;
    sink_unlock();
    return (FT_ERR_SUCCESS);
}

void sink_clear(void) noexcept
{
    sink_node *current_node;
    sink_node *next_node;

    sink_lock();
    current_node = g_sink_head;
    g_sink_head = ft_nullptr;
    g_sink_tail = ft_nullptr;
    g_sink_count = 0;
    sink_unlock();
    while (current_node != ft_nullptr)
    {
        next_node = current_node->next;
        std::free(current_node->message_storage);
        std::free(current_node);
        current_node = next_node;
    }
    return ;
}
