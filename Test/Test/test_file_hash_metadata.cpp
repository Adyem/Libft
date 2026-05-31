#include "../test_internal.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/Filesystem/filesystem.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <cstring>

static ft_bool file_test_digest_equals(const uint8_t *left, const uint8_t *right,
    ft_size_t length)
{
    if (std::memcmp(left, right, length) == 0)
        return (FT_TRUE);
    return (FT_FALSE);
}

static ft_bool file_test_flag_is_set(uint32_t flags, uint32_t flag_value)
{
    if ((flags & flag_value) != 0U)
        return (FT_TRUE);
    return (FT_FALSE);
}

FT_TEST(test_file_hash_sha256_depends_on_contents)
{
    const char *first_path;
    const char *second_path;
    uint8_t first_sha1_digest[20];
    uint8_t second_sha1_digest[20];
    uint8_t first_digest[32];
    uint8_t second_digest[32];
    uint8_t different_digest[32];

    first_path = "test_file_hash_first.txt";
    second_path = "test_file_hash_second.txt";
    (void)file_delete(first_path);
    (void)file_delete(second_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(first_path, "hello", 5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(second_path, "hello", 5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_hash_sha1(first_path, first_sha1_digest));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_hash_sha1(second_path, second_sha1_digest));
    FT_ASSERT_EQ(FT_TRUE, file_test_digest_equals(first_sha1_digest,
            second_sha1_digest, sizeof(first_sha1_digest)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_hash_sha256(first_path, first_digest));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_hash_sha256(second_path, second_digest));
    FT_ASSERT_EQ(FT_TRUE, file_test_digest_equals(first_digest, second_digest,
            sizeof(first_digest)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(second_path, "hello!", 6));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_hash_sha256(second_path, different_digest));
    FT_ASSERT_EQ(FT_FALSE, file_test_digest_equals(first_digest,
            different_digest, sizeof(first_digest)));
    (void)file_delete(first_path);
    (void)file_delete(second_path);
    return (1);
}

FT_TEST(test_file_metadata_diff_reports_type_size_and_missing_changes)
{
    const char *root_path;
    const char *first_file_path;
    const char *second_file_path;
    const char *directory_path;
    const char *missing_path;
    uint32_t difference_flags;

    root_path = "test_file_metadata_diff";
    first_file_path = "test_file_metadata_diff/first.txt";
    second_file_path = "test_file_metadata_diff/second.txt";
    directory_path = "test_file_metadata_diff/directory";
    missing_path = "test_file_metadata_diff/missing.txt";
    (void)file_delete_recursive(root_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_create_directories(directory_path, 0700));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(first_file_path, "abc", 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_write_all(second_file_path, "abcd", 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_metadata_diff(first_file_path,
            first_file_path, &difference_flags));
    FT_ASSERT_EQ(FILE_METADATA_DIFF_NONE, difference_flags);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_metadata_diff(first_file_path,
            second_file_path, &difference_flags));
    FT_ASSERT_EQ(FT_TRUE, file_test_flag_is_set(difference_flags,
            FILE_METADATA_DIFF_SIZE));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_metadata_diff(first_file_path,
            directory_path, &difference_flags));
    FT_ASSERT_EQ(FT_TRUE, file_test_flag_is_set(difference_flags,
            FILE_METADATA_DIFF_TYPE));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_metadata_diff(first_file_path,
            missing_path, &difference_flags));
    FT_ASSERT_EQ(FT_TRUE, file_test_flag_is_set(difference_flags,
            FILE_METADATA_DIFF_MISSING));
    (void)file_delete_recursive(root_path);
    return (1);
}
