#include "libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/nullptr.hpp"

t_stack *ft_stack_new(void)
{
    t_stack *stack = static_cast<t_stack*>(cma_malloc(sizeof(t_stack)));
    if (stack == ft_nullptr)
        return (ft_nullptr);
    stack->top = ft_nullptr;
    stack->size = 0;
    return (stack);
}

int ft_stack_push(t_stack *stack, void *data)
{
    if (stack == ft_nullptr)
        return (FAILURE);
    t_stack_node *node = static_cast<t_stack_node*>(cma_malloc(sizeof(t_stack_node)));
    if (node == ft_nullptr)
        return (FAILURE);
    node->data = data;
    node->next = stack->top;
    stack->top = node;
    stack->size++;
    return (SUCCES);
}

void *ft_stack_pop(t_stack *stack)
{
    if (stack == ft_nullptr || stack->top == ft_nullptr)
        return (ft_nullptr);
    t_stack_node *node = stack->top;
    void *data = node->data;
    stack->top = node->next;
    stack->size--;
    cma_free(node);
    return (data);
}

void *ft_stack_top(t_stack *stack)
{
    if (stack == ft_nullptr || stack->top == ft_nullptr)
        return (ft_nullptr);
    return (stack->top->data);
}

size_t ft_stack_size(t_stack *stack)
{
    if (stack == ft_nullptr)
        return (0);
    return (stack->size);
}

void ft_stack_clear(t_stack *stack, void (*del)(void *))
{
    if (stack == ft_nullptr)
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
}
