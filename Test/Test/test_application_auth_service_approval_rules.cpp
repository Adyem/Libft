#include "../test_internal.hpp"
#include "../../Modules/Application/application.hpp"
#include "../../Modules/Filesystem/filesystem.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

static int32_t cleanup_application_database(const char *file_path)
{
    return (std::remove(file_path));
}

static void create_application_database(const char *file_path)
{
    FILE *file_pointer;

    file_pointer = std::fopen(file_path, "w");
    if (file_pointer == ft_nullptr)
        return ;
    std::fputs("{\n  \"kv_store\": {\n  }\n}\n", file_pointer);
    std::fclose(file_pointer);
    return ;
}

FT_TEST(test_application_auth_service_approval_flags_reset_on_remove)
{
    application_auth_service service;
    ft_string *database_file_path;
    ft_bool approval_required;
    ft_bool approved;
    char root_template[] = "/tmp/libft_application_root_XXXXXX";
    char database_relative_path[] = "auth_db.json";
    char *database_root_path;
    int32_t error_code;

    database_root_path = mkdtemp(root_template);
    FT_ASSERT(database_root_path != ft_nullptr);
    if (database_root_path == ft_nullptr)
        return (1);
    database_file_path = filesystem_safe_join_path(database_root_path, database_relative_path);
    FT_ASSERT(database_file_path != ft_nullptr);
    if (database_file_path == ft_nullptr)
    {
        FT_ASSERT_EQ(0, rmdir(database_root_path));
        return (1);
    }
    create_application_database(database_file_path->c_str());
    error_code = service.initialize(database_root_path, database_relative_path, "app-auth-key-123", FT_TRUE);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, error_code);
    if (error_code != FT_ERR_SUCCESS)
    {
        FT_ASSERT_EQ(0, cleanup_application_database(database_file_path->c_str()));
        FT_ASSERT_EQ(FT_ERR_SUCCESS, database_file_path->destroy());
        delete database_file_path;
        FT_ASSERT_EQ(0, rmdir(database_root_path));
        return (1);
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.register_user("charlie", "pass123"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.set_login_approval_required_for_user("charlie", FT_TRUE));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.is_login_approval_required_for_user("charlie", approval_required));
    FT_ASSERT_EQ(FT_TRUE, approval_required);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.approve_login("charlie"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.is_login_approved("charlie", approved));
    FT_ASSERT_EQ(FT_TRUE, approved);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.remove_user("charlie"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.register_user("charlie", "pass123"));
    approval_required = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.is_login_approval_required_for_user("charlie", approval_required));
    FT_ASSERT_EQ(FT_FALSE, approval_required);
    approved = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.is_login_approved("charlie", approved));
    FT_ASSERT_EQ(FT_FALSE, approved);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.destroy());
    FT_ASSERT_EQ(0, cleanup_application_database(database_file_path->c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, database_file_path->destroy());
    delete database_file_path;
    FT_ASSERT_EQ(0, rmdir(database_root_path));
    return (1);
}

FT_TEST(test_application_auth_service_global_override_enforces_approval)
{
    application_auth_service service;
    ft_string *database_file_path;
    ft_bool authenticated;
    char root_template[] = "/tmp/libft_application_root_XXXXXX";
    char database_relative_path[] = "auth_db.json";
    char *database_root_path;
    int32_t error_code;

    database_root_path = mkdtemp(root_template);
    FT_ASSERT(database_root_path != ft_nullptr);
    if (database_root_path == ft_nullptr)
        return (1);
    database_file_path = filesystem_safe_join_path(database_root_path, database_relative_path);
    FT_ASSERT(database_file_path != ft_nullptr);
    if (database_file_path == ft_nullptr)
    {
        FT_ASSERT_EQ(0, rmdir(database_root_path));
        return (1);
    }
    create_application_database(database_file_path->c_str());
    error_code = service.initialize(database_root_path, database_relative_path, "app-auth-key-123", FT_TRUE);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, error_code);
    if (error_code != FT_ERR_SUCCESS)
    {
        FT_ASSERT_EQ(0, cleanup_application_database(database_file_path->c_str()));
        FT_ASSERT_EQ(FT_ERR_SUCCESS, database_file_path->destroy());
        delete database_file_path;
        FT_ASSERT_EQ(0, rmdir(database_root_path));
        return (1);
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.register_user("dana", "s3cret"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.authenticate_user("dana", "s3cret", authenticated));
    FT_ASSERT_EQ(FT_TRUE, authenticated);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.set_manual_login_approval_enabled(FT_TRUE));
    authenticated = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_PERMISSION_DENIED, service.authenticate_user("dana", "s3cret", authenticated));
    FT_ASSERT_EQ(FT_FALSE, authenticated);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.approve_login("dana"));
    authenticated = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.authenticate_user("dana", "s3cret", authenticated));
    FT_ASSERT_EQ(FT_TRUE, authenticated);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.destroy());
    FT_ASSERT_EQ(0, cleanup_application_database(database_file_path->c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, database_file_path->destroy());
    delete database_file_path;
    FT_ASSERT_EQ(0, rmdir(database_root_path));
    return (1);
}
