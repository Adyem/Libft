#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/nullptr.hpp"

int test_queue_basic(void)
{
    t_queue *queue = ft_queue_new();
    if (!queue)
        return (0);
    int a = 1;
    int b = 2;
    if (ft_queue_enqueue(queue, &a) != SUCCES)
    {
        ft_queue_clear(queue, ft_nullptr);
        cma_free(queue);
        return (0);
    }
    if (ft_queue_enqueue(queue, &b) != SUCCES)
    {
        ft_queue_clear(queue, ft_nullptr);
        cma_free(queue);
        return (0);
    }
    if (*(int *)ft_queue_front(queue) != 1)
    {
        ft_queue_clear(queue, ft_nullptr);
        cma_free(queue);
        return (0);
    }
    int *first = static_cast<int *>(ft_queue_dequeue(queue));
    if (!first || *first != 1)
    {
        ft_queue_clear(queue, ft_nullptr);
        cma_free(queue);
        return (0);
    }
    int *second = static_cast<int *>(ft_queue_dequeue(queue));
    if (!second || *second != 2)
    {
        ft_queue_clear(queue, ft_nullptr);
        cma_free(queue);
        return (0);
    }
    int *none = static_cast<int *>(ft_queue_dequeue(queue));
    if (none != ft_nullptr)
    {
        ft_queue_clear(queue, ft_nullptr);
        cma_free(queue);
        return (0);
    }
    if (ft_queue_size(queue) != 0)
    {
        ft_queue_clear(queue, ft_nullptr);
        cma_free(queue);
        return (0);
    }
    ft_queue_clear(queue, ft_nullptr);
    cma_free(queue);
    return (1);
}

