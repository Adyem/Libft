#include "libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/nullptr.hpp"
#include <new>


t_queue *ft_queue_new(void)
{
    t_queue *queue = static_cast<t_queue*>(cma_malloc(sizeof(t_queue)));
    if (queue == ft_nullptr)
        return (ft_nullptr);
    new (&queue->mutex) pt_mutex();
    queue->front = ft_nullptr;
    queue->rear = ft_nullptr;
    queue->size = 0;
    return (queue);
}

int ft_queue_enqueue(t_queue *queue, void *data)
{
    if (queue == ft_nullptr)
        return (FAILURE);
    if (queue->mutex.lock(THREAD_ID) != SUCCES)
        return (FAILURE);
    t_queue_node *node = static_cast<t_queue_node*>(cma_malloc(sizeof(t_queue_node)));
    if (node == ft_nullptr)
    {
        queue->mutex.unlock(THREAD_ID);
        return (FAILURE);
    }
    node->data = data;
    node->next = ft_nullptr;
    if (queue->rear == ft_nullptr)
    {
        queue->front = node;
        queue->rear = node;
    }
    else
    {
        queue->rear->next = node;
        queue->rear = node;
    }
    queue->size++;
    queue->mutex.unlock(THREAD_ID);
    return (SUCCES);
}

void *ft_queue_dequeue(t_queue *queue)
{
    if (queue == ft_nullptr)
        return (ft_nullptr);
    if (queue->mutex.lock(THREAD_ID) != SUCCES)
        return (ft_nullptr);
    if (queue->front == ft_nullptr)
    {
        queue->mutex.unlock(THREAD_ID);
        return (ft_nullptr);
    }
    t_queue_node *node = queue->front;
    void *data = node->data;
    queue->front = node->next;
    if (queue->front == ft_nullptr)
        queue->rear = ft_nullptr;
    queue->size--;
    queue->mutex.unlock(THREAD_ID);
    cma_free(node);
    return (data);
}

void *ft_queue_front(t_queue *queue)
{
    if (queue == ft_nullptr)
        return (ft_nullptr);
    if (queue->mutex.lock(THREAD_ID) != SUCCES)
        return (ft_nullptr);
    if (queue->front == ft_nullptr)
    {
        queue->mutex.unlock(THREAD_ID);
        return (ft_nullptr);
    }
    void *data = queue->front->data;
    queue->mutex.unlock(THREAD_ID);
    return (data);
}

size_t ft_queue_size(t_queue *queue)
{
    if (queue == ft_nullptr)
        return (0);
    if (queue->mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t size = queue->size;
    queue->mutex.unlock(THREAD_ID);
    return (size);
}

void ft_queue_clear(t_queue *queue, void (*del)(void *))
{
    if (queue == ft_nullptr)
        return;
    if (queue->mutex.lock(THREAD_ID) != SUCCES)
        return;
    t_queue_node *node = queue->front;
    while (node != ft_nullptr)
    {
        t_queue_node *next = node->next;
        if (del)
            del(node->data);
        cma_free(node);
        node = next;
    }
    queue->front = ft_nullptr;
    queue->rear = ft_nullptr;
    queue->size = 0;
    queue->mutex.unlock(THREAD_ID);
}

