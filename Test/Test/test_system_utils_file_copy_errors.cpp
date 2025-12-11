#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstdio>

FT_TEST(test_su_copy_file_rejects_null_source,
        "su_copy_file rejects null source path arguments")
{
    const char *destination_path;
    FILE *destination_file;

    destination_path = "su_copy_file_null_source.txt";
    std::remove(destination_path);

    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, su_copy_file(ft_nullptr, destination_path));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);

    destination_file = std::fopen(destination_path, "r");
    FT_ASSERT(destination_file == ft_nullptr);
    if (destination_file != ft_nullptr)
        std::fclose(destination_file);
    std::remove(destination_path);
    return (1);
}

FT_TEST(test_su_copy_file_missing_source_sets_error,
        "su_copy_file reports errors when the source file is missing")
{
    const char *source_path;
    const char *destination_path;
    FILE *destination_file;

    source_path = "su_copy_file_missing_source.txt";
    destination_path = "su_copy_file_missing_destination.txt";
    std::remove(source_path);
    std::remove(destination_path);

    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, su_copy_file(source_path, destination_path));
    FT_ASSERT(ft_errno != FT_ERR_SUCCESSS);

    destination_file = std::fopen(destination_path, "r");
    FT_ASSERT(destination_file == ft_nullptr);
    if (destination_file != ft_nullptr)
        std::fclose(destination_file);
    std::remove(destination_path);
    return (1);
}
