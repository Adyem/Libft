#include "../test_internal.hpp"
#include "../../Modules/Application/application.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <cstdlib>
#include <unistd.h>
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Storage/kv_store.hpp"
#include "../../Modules/Template/pair.hpp"
#include "../../Modules/Template/vector.hpp"

static int32_t cleanup_application_root(char *root_path)
{
    return (rmdir(root_path));
}

FT_TEST(test_application_auth_service_rejects_path_escape)
{
    application_auth_service service;
    char root_template[] = "/tmp/libft_application_root_XXXXXX";
    char *database_root_path;
    int32_t error_code;

    database_root_path = mkdtemp(root_template);
    FT_ASSERT(database_root_path != ft_nullptr);
    if (database_root_path == ft_nullptr)
        return (1);
    error_code = service.initialize(database_root_path, "../escape.json", "app-auth-key-123", FT_TRUE);
    FT_ASSERT_EQ(FT_ERR_INVALID_PATH, error_code);
    FT_ASSERT_EQ(FT_FALSE, service.is_initialised());
    FT_ASSERT_EQ(0, cleanup_application_root(database_root_path));
    return (1);
}

FT_TEST(test_application_auth_service_rejects_absolute_database_path)
{
    application_auth_service service;
    char root_template[] = "/tmp/libft_application_root_XXXXXX";
    char *database_root_path;
    int32_t error_code;

    database_root_path = mkdtemp(root_template);
    FT_ASSERT(database_root_path != ft_nullptr);
    if (database_root_path == ft_nullptr)
        return (1);
    error_code = service.initialize(database_root_path, "/tmp/libft_application_auth.json", "app-auth-key-123", FT_TRUE);
    FT_ASSERT_EQ(FT_ERR_INVALID_PATH, error_code);
    FT_ASSERT_EQ(FT_FALSE, service.is_initialised());
    FT_ASSERT_EQ(0, cleanup_application_root(database_root_path));
    return (1);
}
