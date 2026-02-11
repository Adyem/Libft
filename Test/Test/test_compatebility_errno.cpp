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
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_set_errno_from_system_error_records_mapping,
    "ft_set_errno_from_system_error stores mapped errno")
{
    int result;

    ft_errno = FT_ERR_SUCCESSS;
    result = ft_set_errno_from_system_error(ENOMEM);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, result);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    return (1);
}

FT_TEST(test_cmp_decode_errno_offset_error_decodes_offset_encoding_and_resets_errno,
    "cmp_decode_errno_offset_error converts offset encoded errors and clears errno")
{
    int normalized_error;

    ft_errno = FT_ERR_CONFIGURATION;
    normalized_error = cmp_decode_errno_offset_error(ERRNO_OFFSET + ENOMEM);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, normalized_error);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_TERMINATED;
    normalized_error = cmp_decode_errno_offset_error(ERRNO_OFFSET);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, normalized_error);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_ALREADY_EXISTS;
    normalized_error = cmp_decode_errno_offset_error(FT_ERR_IO);
    FT_ASSERT_EQ(FT_ERR_IO, normalized_error);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}
