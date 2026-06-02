#include "../test_internal.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/system_utils.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <cstdio>

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_su_copy_file_rejects_null_source)
{
    const char *destination_path;
    FILE *destination_file;

    destination_path = "su_copy_file_null_source.txt";
    std::remove(destination_path);
    FT_ASSERT_EQ(-1, su_copy_file(ft_nullptr, destination_path));

    destination_file = std::fopen(destination_path, "r");
    FT_ASSERT(destination_file == ft_nullptr);
    if (destination_file != ft_nullptr)
        std::fclose(destination_file);
    std::remove(destination_path);
    return (1);
}

FT_TEST(test_su_copy_file_missing_source_sets_error)
{
    const char *source_path;
    const char *destination_path;
    FILE *destination_file;

    source_path = "su_copy_file_missing_source.txt";
    destination_path = "su_copy_file_missing_destination.txt";
    std::remove(source_path);
    std::remove(destination_path);
    FT_ASSERT_EQ(-1, su_copy_file(source_path, destination_path));

    destination_file = std::fopen(destination_path, "r");
    FT_ASSERT(destination_file == ft_nullptr);
    if (destination_file != ft_nullptr)
        std::fclose(destination_file);
    std::remove(destination_path);
    return (1);
}
