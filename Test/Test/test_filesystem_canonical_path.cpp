#include "../test_internal.hpp"
#include "../../Modules/Filesystem/filesystem.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_filesystem_canonical_path_returns_absolute_existing_path)
{
    ft_string *canonical_path;

    canonical_path = filesystem_canonical_path(".");
    FT_ASSERT(canonical_path != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, canonical_path->get_error());
    FT_ASSERT_EQ(FT_TRUE, filesystem_is_absolute(canonical_path->c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, canonical_path->destroy());
    delete canonical_path;
    canonical_path = filesystem_canonical_path(
        "definitely_missing_libft_path_entry");
    FT_ASSERT(canonical_path == ft_nullptr);
    return (1);
}
