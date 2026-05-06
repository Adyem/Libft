#include "../CPP_class/class_nullptr.hpp"
#include "system_utils.hpp"
#include <cstdlib>

const char  *su_internal_take_abort_reason(void);

void    su_abort(void)
{
    const char  *reason;

    reason = su_internal_take_abort_reason();
    if (reason == ft_nullptr)
        reason = "su_abort invoked";
    su_run_resource_tracers(reason);
    std::abort();
    return ;
}
