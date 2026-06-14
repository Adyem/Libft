#include "errno_internal.hpp"
#include "errno.hpp"
#include "../Basic/basic.hpp"
#include "../Basic/class_nullptr.hpp"
#include <cstdio>

namespace
{
    static void write_uint32_decimal(uint32_t value) noexcept
    {
        char decimal_buffer[10];
        ft_size_t digit_count;

        if (value == 0U)
        {
            errno_write_stderr("0");
            return ;
        }
        digit_count = 0;
        while (value > 0U && digit_count < sizeof(decimal_buffer))
        {
            decimal_buffer[digit_count] = static_cast<char>('0' + (value % 10U));
            value /= 10U;
            digit_count++;
        }
        while (digit_count > 0U)
        {
            digit_count--;
            (void)su_write(2, &decimal_buffer[digit_count], 1U);
        }
        return ;
    }
}

void    errno_write_stderr(const char *string) noexcept
{
    if (string == ft_nullptr)
        return ;
    (void)std::fwrite(string, 1, ft_strlen_size_t(string), stderr);
    (void)std::fflush(stderr);
    return ;
}

void    errno_abort_lifecycle(uint8_t initialised_state, const char *method_name,
            const char *reason)
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    errno_write_stderr("lifecycle error: state=");
    write_uint32_decimal(static_cast<uint32_t>(initialised_state));
    errno_write_stderr(" method=");
    errno_write_stderr(method_name);
    errno_write_stderr(" reason=");
    errno_write_stderr(reason);
    errno_write_stderr("\n");
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
