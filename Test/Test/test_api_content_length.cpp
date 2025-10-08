#include "../../API/api_internal.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_api_append_content_length_propagates_string_error, "api_append_content_length_header returns false when ft_string append fails")
{
    ft_string request;
    bool append_result;

    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(1);
    append_result = api_append_content_length_header(request, 42);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(false, append_result);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, request.get_error());
    return (1);
}

FT_TEST(test_api_append_content_length_successful_append, "api_append_content_length_header appends header on success")
{
    ft_string request;
    bool append_result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    append_result = api_append_content_length_header(request, 7);
    FT_ASSERT_EQ(true, append_result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(request.find("Content-Length: 7") != ft_string::npos);
    return (1);
}

