# ReadLine resize test failure analysis

The `test_readline_printable_char_preserves_buffer_on_resize_failure` regression triggers
`cma_set_alloc_limit(1)` before calling `rl_handle_printable_char`. That limit forces
`rl_resize_buffer` to walk the allocation failure path and emit "Allocation error".

Originally the helper logged the message through `pf_printf_fd`, which routed the string
into the Printf formatting engine. That engine tokenises the message into dynamically
allocated `std::vector` instances. With the allocator limit clamped to a single byte,
those allocations failed, the C++ runtime raised `std::bad_alloc`, and the project—built
without exception handling—called `std::terminate`, aborting the test before the resize
helper could return `-1`.

The helper now writes the message with `su_write`, which avoids any dynamic allocations.
As a result, the failure path cleanly reports `FT_ERR_NO_MEMORY` and preserves the caller
buffer so the regression test can verify the behaviour.
