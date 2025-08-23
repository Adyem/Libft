#include "libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/nullptr.hpp"
#include <new>

t_stack *ft_stack_new(void)
{
    t_stack *stack = static_cast<t_stack*>(cma_malloc(sizeof(t_stack)));
    if (stack == ft_nullptr)
        return (ft_nullptr);
    new (&stack->mutex) pt_mutex();
    stack->top = ft_nullptr;
    stack->size = 0;
    return (stack);
}

int ft_stack_push(t_stack *stack, void *data)
{
    if (stack == ft_nullptr)
        return (FAILURE);
    if (stack->mutex.lock(THREAD_ID) != SUCCES)
        return (FAILURE);
    t_stack_node *node = static_cast<t_stack_node*>(cma_malloc(sizeof(t_stack_node)));
    if (node == ft_nullptr)
    {
        stack->mutex.unlock(THREAD_ID);
        return (FAILURE);
    }
    node->data = data;
    node->next = stack->top;
    stack->top = node;
    stack->size++;
    stack->mutex.unlock(THREAD_ID);
    return (SUCCES);
}

void *ft_stack_pop(t_stack *stack)
{
    if (stack == ft_nullptr)
        return (ft_nullptr);
    if (stack->mutex.lock(THREAD_ID) != SUCCES)
        return (ft_nullptr);
    if (stack->top == ft_nullptr)
    {
        stack->mutex.unlock(THREAD_ID);
        return (ft_nullptr);
    }
    t_stack_node *node = stack->top;
    void *data = node->data;
    stack->top = node->next;
    stack->size--;
    stack->mutex.unlock(THREAD_ID);
    cma_free(node);
    return (data);
}

void *ft_stack_top(t_stack *stack)
{
    if (stack == ft_nullptr)
        return (ft_nullptr);
    if (stack->mutex.lock(THREAD_ID) != SUCCES)
        return (ft_nullptr);
    if (stack->top == ft_nullptr)
    {
        stack->mutex.unlock(THREAD_ID);
        return (ft_nullptr);
    }
    void *data = stack->top->data;
    stack->mutex.unlock(THREAD_ID);
    return (data);
}

size_t ft_stack_size(t_stack *stack)
{
    if (stack == ft_nullptr)
        return (0);
    if (stack->mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t size = stack->size;
    stack->mutex.unlock(THREAD_ID);
    return (size);
}

void ft_stack_clear(t_stack *stack, void (*del)(void *))
{
    if (stack == ft_nullptr)
        return;
    if (stack->mutex.lock(THREAD_ID) != SUCCES)
        return;
    t_stack_node *node = stack->top;
    while (node != ft_nullptr)
    {
        t_stack_node *next = node->next;
        if (del)
            del(node->data);
        cma_free(node);
        node = next;
    }
    stack->top = ft_nullptr;
    stack->size = 0;
    stack->mutex.unlock(THREAD_ID);
}
