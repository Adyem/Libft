#include "errno_internal.hpp"
#include "errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

void    errno_abort_lifecycle(uint8_t initialised_state, const char *method_name,
            const char *reason)
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "lifecycle error: state=%u method=%s reason=%s\n",
        static_cast<uint32_t>(initialised_state), method_name, reason);
    su_abort();
    return ;
}

void    errno_abort_if_uninitialised(uint8_t initialised_state,
            const char *method_name)
{
    if (initialised_state != FT_CLASS_STATE_UNINITIALISED)
    {
        return ;
    }
    errno_abort_lifecycle(initialised_state, method_name,
        "called while object is uninitialised");
    return ;
}

void    errno_abort_if_uninitialised_or_destroyed(uint8_t initialised_state,
            const char *method_name)
{
    if (initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        return ;
    }
    errno_abort_lifecycle(initialised_state, method_name,
        "called while object is uninitialised or destroyed");
    return ;
}
