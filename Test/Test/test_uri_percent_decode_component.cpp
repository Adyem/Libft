#include "../test_internal.hpp"
#include "../../Modules/URI/uri.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_uri_percent_decode_component_decodes_escaped_bytes)
{
    const uint8_t expected_bytes[] = {'a', ' ', '/', '~'};
    uint8_t *percent_decoded_component;
    ft_size_t percent_decoded_size;

    percent_decoded_size = 0;
    percent_decoded_component = uri_percent_decode_component("a%20%2F~", 8,
            &percent_decoded_size);
    FT_ASSERT(percent_decoded_component != ft_nullptr);
    FT_ASSERT_EQ(4, percent_decoded_size);
    FT_ASSERT_EQ(0, ft_memcmp(expected_bytes, percent_decoded_component,
        percent_decoded_size));
    cma_free(percent_decoded_component);
    return (1);
}
