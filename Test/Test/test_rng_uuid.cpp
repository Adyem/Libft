#include "../../RNG/rng.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

static int is_lower_hexadecimal_character(char value)
{
    if (value >= '0' && value <= '9')
        return (1);
    if (value >= 'a' && value <= 'f')
        return (1);
    return (0);
}

FT_TEST(test_ft_generate_uuid_null_out_sets_errno, "ft_generate_uuid null output sets errno")
{
    ft_errno = FT_ERR_SUCCESSS;
    ft_generate_uuid(ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_ft_generate_uuid_success_sets_errno_success, "ft_generate_uuid success sets errno success")
{
    char output[37];
    size_t index;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_generate_uuid(output);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    index = 0;
    while (index < 36)
    {
        if (index == 8 || index == 13 || index == 18 || index == 23)
            FT_ASSERT_EQ('-', output[index]);
        else
            FT_ASSERT(is_lower_hexadecimal_character(output[index]) != 0);
        index++;
    }
    FT_ASSERT_EQ('\0', output[36]);
    return (1);
}
