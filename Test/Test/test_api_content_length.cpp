#include "../test_internal.hpp"
#include "../../API/api_internal.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_api_append_content_length_propagates_string_error)
{
    ft_string request;
    bool append_result;

    cma_set_alloc_limit(1);
    append_result = api_append_content_length_header(request, 42);
    FT_ASSERT_EQ(false, append_result);
    FT_ASSERT_EQ(static_cast<uint32_t>(FT_ERR_NO_MEMORY), ft_string::get_error());
    cma_set_alloc_limit(0);
    return (1);
}

FT_TEST(test_api_append_content_length_successful_append)
{
    ft_string request;
    bool append_result;

    append_result = api_append_content_length_header(request, 7);
    FT_ASSERT_EQ(true, append_result);
    FT_ASSERT(request.find("Content-Length: 7") != ft_string::npos);
    return (1);
}
