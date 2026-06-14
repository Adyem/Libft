#include "../Basic/class_nullptr.hpp"
#include "../Compatebility/compatebility_stack_trace.hpp"
#include "system_utils.hpp"
#include <cstdlib>
#include <cstdio>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <csignal>

const char  *su_internal_take_abort_reason(void);

void    su_abort(void)
{
#ifndef LIBFT_TEST_BUILD
    void        *stack_frames[CMP_STACK_TRACE_MAX_FRAMES];
    ft_size_t   frame_count;
#endif
    const char  *reason;

    reason = su_internal_take_abort_reason();
    if (reason == ft_nullptr)
        reason = "su_abort invoked";
#ifndef LIBFT_TEST_BUILD
    su_run_resource_tracers(reason);
#endif
    std::fprintf(stderr, "libft abort: %s\n", reason);
#ifndef LIBFT_TEST_BUILD
    frame_count = cmp_stack_trace_capture(stack_frames,
            CMP_STACK_TRACE_MAX_FRAMES, 1);
    if (frame_count > 0)
    {
        std::fprintf(stderr, "libft abort: stack trace:\n");
        cmp_stack_trace_print(stderr, stack_frames, frame_count);
    }
#endif
    std::fflush(nullptr);
    (void)std::raise(SIGABRT);
    std::abort();
    return ;
}
