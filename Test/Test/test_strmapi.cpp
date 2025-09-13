#include "../../Libft/libft.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"

static char to_upper_map(unsigned int index, char character)
{
    (void)index;
    if (character >= 'a' && character <= 'z')
        return (character - 32);
    return (character);
}

FT_TEST(test_strmapi_basic, "ft_strmapi basic")
{
    char *result;

    result = ft_strmapi("ab", to_upper_map);
    if (result == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp("AB", result));
    cma_free(result);

    result = ft_strmapi("", to_upper_map);
    if (result == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp("", result));
    cma_free(result);

    FT_ASSERT_EQ(ft_nullptr, ft_strmapi(ft_nullptr, to_upper_map));
    FT_ASSERT_EQ(ft_nullptr, ft_strmapi("ab", ft_nullptr));
    return (1);
}
