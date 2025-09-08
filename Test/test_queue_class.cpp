#include "../Template/template_queue.hpp"
#include "../Errno/errno.hpp"

int test_ft_queue_class_basic(void)
{
    ft_queue<int> q;
    q.enqueue(1);
    q.enqueue(2);
    if (q.get_error() != ER_SUCCESS || q.front() != 1 || q.size() != 2)
        return (0);
    int first = q.dequeue();
    if (first != 1)
        return (0);
    int second = q.dequeue();
    if (second != 2)
        return (0);
    q.dequeue();
    if (q.get_error() != QUEUE_EMPTY)
        return (0);
    return (1);
}

