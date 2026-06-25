# Command

The `Command` module implements an undo/redo stack around caller-provided command callbacks. The stack uses caller-owned arrays for undo and redo storage.

## Types

- `command_callback` - Function pointer used for execute and undo operations. It receives a caller context pointer.
- `command_action` - One command entry. Public fields are `name`, `execute`, `undo`, and `context`.
- `command_stack` - Undo/redo storage. Public fields hold the undo and redo arrays, capacities, and counts.

## Public API

- `command_stack_initialize(command_stack *stack, command_action *undo_entries, ft_size_t undo_capacity, command_action *redo_entries, ft_size_t redo_capacity)` - Binds a stack to caller-provided storage and resets counts.
- `command_stack_clear(command_stack *stack)` - Clears undo and redo history.
- `command_stack_can_undo(const command_stack *stack)` - Reports whether an undo action is available.
- `command_stack_can_redo(const command_stack *stack)` - Reports whether a redo action is available.
- `command_stack_undo_count(const command_stack *stack)` - Returns the number of undo entries.
- `command_stack_redo_count(const command_stack *stack)` - Returns the number of redo entries.
- `command_stack_execute(command_stack *stack, const command_action *action)` - Runs an action, pushes it onto undo history, and clears redo history.
- `command_stack_undo(command_stack *stack)` - Runs the top action's undo callback and moves it to redo history.
- `command_stack_redo(command_stack *stack)` - Re-runs the top redo action and moves it back to undo history.
- `command_stack_peek_undo(const command_stack *stack)` - Returns the action that would be undone next.
- `command_stack_peek_redo(const command_stack *stack)` - Returns the action that would be redone next.
