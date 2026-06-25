#ifndef COMMAND_HPP
# define COMMAND_HPP

#include "../Errno/errno.hpp"

typedef int32_t (*command_callback)(void *context);

struct command_action
{
    const char       *name;
    command_callback execute;
    command_callback undo;
    void             *context;
};

struct command_stack
{
    command_action   *undo_entries;
    ft_size_t        undo_capacity;
    ft_size_t        undo_count;
    command_action   *redo_entries;
    ft_size_t        redo_capacity;
    ft_size_t        redo_count;
};

int32_t     command_stack_initialize(command_stack *stack,
                command_action *undo_entries, ft_size_t undo_capacity,
                command_action *redo_entries, ft_size_t redo_capacity);
void        command_stack_clear(command_stack *stack);
ft_bool     command_stack_can_undo(const command_stack *stack);
ft_bool     command_stack_can_redo(const command_stack *stack);
ft_size_t   command_stack_undo_count(const command_stack *stack);
ft_size_t   command_stack_redo_count(const command_stack *stack);
int32_t     command_stack_execute(command_stack *stack,
                const command_action *action);
int32_t     command_stack_undo(command_stack *stack);
int32_t     command_stack_redo(command_stack *stack);
const command_action *command_stack_peek_undo(const command_stack *stack);
const command_action *command_stack_peek_redo(const command_stack *stack);

#endif
