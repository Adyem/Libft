#include "../Template/template_queue.hpp"

int test_queue_basic(void)
{
    ft_queue<int> queue;
    int value_a = 1;
    int value_b = 2;
    queue.enqueue(value_a);
    queue.enqueue(value_b);
    if (queue.front() != 1)
        return (0);
    int first = queue.dequeue();
    if (first != 1)
        return (0);
    int second = queue.dequeue();
    if (second != 2)
        return (0);
    if (!queue.empty())
        return (0);
    return (1);
}

