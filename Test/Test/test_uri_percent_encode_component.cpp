#include "../test_internal.hpp"
#include "../../Modules/URI/uri.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_uri_percent_encode_component_encodes_reserved_bytes)
{
    const uint8_t original_bytes[] = {'a', ' ', '/', '~'};
    char *percent_encoded_component;

    percent_encoded_component = uri_percent_encode_component(original_bytes, 4,
            FT_FALSE);
    FT_ASSERT(percent_encoded_component != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(percent_encoded_component, "a%20%2F~"));
    cma_free(percent_encoded_component);
    return (1);
}
