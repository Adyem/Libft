#include "pair.hpp"
#include "queue.hpp"
#include "stack.hpp"
#include "vector.hpp"

namespace template_compile
{
static int build_types()
{
    ft_vector<int> integers;

    integers.push_back(3);
    integers.push_back(7);
    if (!integers.empty())
        integers.pop_back();
    Pair<int, long> pair_value(42, 13L);
    ft_vector<Pair<int, long>> pair_collection;
    pair_collection.push_back(pair_value);
    ft_queue<int> queue_values;
    queue_values.enqueue(5);
    queue_values.enqueue(10);
    if (!queue_values.empty())
        queue_values.dequeue();
    ft_stack<double> stack_values;
    stack_values.push(1.5);
    stack_values.push(2.5);
    if (!stack_values.empty())
        stack_values.pop();
    int queue_head = queue_values.front();
    double stack_top = stack_values.top();
    (void)queue_head;
    (void)stack_top;
    return static_cast<int>(integers.size()
            + pair_collection.size()
            + queue_values.size()
            + stack_values.size());
}

static volatile int template_basic_types_result = build_types();
}
