#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"

FT_TEST(test_file_write_all_rejects_null_payload_with_size)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, file_write_all("test_invalid.txt", ft_nullptr, 1));
    return (1);
}
