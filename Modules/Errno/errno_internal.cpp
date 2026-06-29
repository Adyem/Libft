#include "errno_internal.hpp"
#include "errno.hpp"
#include "../Basic/basic.hpp"
#include "../Basic/class_nullptr.hpp"
#include <cstdio>

namespace
{
    static int32_t write_uint32_decimal(uint32_t value) noexcept
    {
        char decimal_buffer[10];
        ft_size_t digit_count;

        if (value == 0U)
        {
            return (errno_write_stderr("0"));
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
            if (su_write(2, &decimal_buffer[digit_count], 1U) != 1)
                return (FT_ERR_SYSTEM);
        }
        return (FT_ERR_SUCCESS);
    }
}

int32_t errno_write_stderr(const char *string) noexcept
{
    size_t write_length;
    size_t write_result;

    if (string == ft_nullptr)
        return (FT_ERR_INVALID_POINTER);
    write_length = ft_strlen_size_t(string);
    write_result = std::fwrite(string, 1, write_length, stderr);
    if (write_result != write_length)
        return (FT_ERR_SYSTEM);
    if (std::fflush(stderr) != 0)
        return (FT_ERR_SYSTEM);
    return (FT_ERR_SUCCESS);
}

void    errno_abort_lifecycle(uint8_t initialised_state, const char *method_name,
            const char *reason)
{
    ft_bool suppress_output;
    int32_t output_error;

    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
#ifdef LIBFT_TEST_BUILD
    suppress_output = FT_TRUE;
#else
    suppress_output = FT_FALSE;
#endif
    output_error = FT_ERR_SUCCESS;
    if (suppress_output == FT_FALSE)
    {
        output_error = errno_write_stderr("lifecycle error: state=");
        if (output_error == FT_ERR_SUCCESS)
            output_error = write_uint32_decimal(static_cast<uint32_t>(initialised_state));
        if (output_error == FT_ERR_SUCCESS)
            output_error = errno_write_stderr(" method=");
        if (output_error == FT_ERR_SUCCESS)
            output_error = errno_write_stderr(method_name);
        if (output_error == FT_ERR_SUCCESS)
            output_error = errno_write_stderr(" reason=");
        if (output_error == FT_ERR_SUCCESS)
            output_error = errno_write_stderr(reason);
        if (output_error == FT_ERR_SUCCESS)
            output_error = errno_write_stderr("\n");
    }
    (void)output_error;
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
