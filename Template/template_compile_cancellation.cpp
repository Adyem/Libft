#include "cancellation.hpp"

static int compile_cancellation_usage()
{
    ft_cancellation_state cancellation_state;

    if (cancellation_state.initialize() != FT_ERR_SUCCESS)
        return (0);
    (void)cancellation_state.request_cancel();
    if (cancellation_state.destroy() != FT_ERR_SUCCESS)
        return (0);
    return (1);
}

static int cancellation_dummy = compile_cancellation_usage();
