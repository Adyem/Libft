#include "../../Compatebility/compatebility_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstring>

FT_TEST(test_cmp_path_separator_matches_platform, "cmp_path_separator returns platform separator")
{
#if defined(_WIN32) || defined(_WIN64)
    const char expected_separator = '\\';
#else
    const char expected_separator = '/';
#endif
    FT_ASSERT_EQ(expected_separator, cmp_path_separator());
    return (1);
}

FT_TEST(test_cmp_normalize_slashes_translates_alternate_separator, "cmp_normalize_slashes converts alternate separator characters")
{
#if defined(_WIN32) || defined(_WIN64)
    char path[] = "folder/subdir/file.txt";
    cmp_normalize_slashes(path);
    FT_ASSERT_EQ(0, std::strcmp("folder\\subdir\\file.txt", path));
#else
    char path[] = "folder\\subdir\\file.txt";
    cmp_normalize_slashes(path);
    FT_ASSERT_EQ(0, std::strcmp("folder/subdir/file.txt", path));
#endif
    return (1);
}

FT_TEST(test_cmp_normalize_slashes_preserves_native_separator, "cmp_normalize_slashes leaves native separator untouched")
{
#if defined(_WIN32) || defined(_WIN64)
    char path[] = "folder\\native\\path";
    cmp_normalize_slashes(path);
    FT_ASSERT_EQ(0, std::strcmp("folder\\native\\path", path));
#else
    char path[] = "folder/native/path";
    cmp_normalize_slashes(path);
    FT_ASSERT_EQ(0, std::strcmp("folder/native/path", path));
#endif
    return (1);
}

FT_TEST(test_cmp_normalize_slashes_handles_empty_string, "cmp_normalize_slashes handles empty strings without modification")
{
    char path[] = "";

    cmp_normalize_slashes(path);
    FT_ASSERT_EQ(0, std::strcmp("", path));
    return (1);
}
