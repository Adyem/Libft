#include "../../Errno/errno.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cerrno>

FT_TEST(test_ft_map_system_error_translates_and_resets_errno,
    "ft_map_system_error translates permission errors and clears errno")
{
    int mapped_error;

    ft_errno = FT_ERR_TERMINATED;
    mapped_error = ft_map_system_error(EACCES);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, mapped_error);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_set_errno_from_system_error_records_mapping,
    "ft_set_errno_from_system_error stores mapped errno")
{
    int result;

    ft_errno = ER_SUCCESS;
    result = ft_set_errno_from_system_error(ENOMEM);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, result);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_cmp_normalize_ft_errno_decodes_offset_encoding_and_resets_errno,
    "cmp_normalize_ft_errno converts offset encoded errors and clears errno")
{
    int normalized_error;

    ft_errno = FT_ERR_CONFIGURATION;
    normalized_error = cmp_normalize_ft_errno(ERRNO_OFFSET + ENOMEM);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, normalized_error);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FT_ERR_TERMINATED;
    normalized_error = cmp_normalize_ft_errno(ERRNO_OFFSET);
    FT_ASSERT_EQ(ER_SUCCESS, normalized_error);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FT_ERR_ALREADY_EXISTS;
    normalized_error = cmp_normalize_ft_errno(FT_ERR_IO);
    FT_ASSERT_EQ(FT_ERR_IO, normalized_error);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
