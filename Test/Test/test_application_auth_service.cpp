#include "../test_internal.hpp"
#include "../../Modules/Application/application.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <cstdio>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

static void cleanup_application_database(const char *file_path)
{
    std::remove(file_path);
    return ;
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

FT_TEST(test_application_auth_service_register_and_authenticate)
{
    application_auth_service service;
    ft_bool authenticated;
    ft_bool exists;
    ft_bool approval_enabled;
    ft_bool approval_required;
    char file_template[] = "/tmp/libft_application_auth_XXXXXX";
    int file_descriptor;
    int32_t error_code;

    file_descriptor = mkstemp(file_template);
    FT_ASSERT(file_descriptor >= 0);
    if (file_descriptor >= 0)
        close(file_descriptor);
    create_application_database(file_template);
    error_code = service.initialize(file_template, "app-auth-key-123", FT_TRUE);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, error_code);
    if (error_code != FT_ERR_SUCCESS)
    {
        cleanup_application_database(file_template);
        return (1);
    }
    FT_ASSERT_EQ(FT_TRUE, service.is_initialised());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.register_user("alice", "s3cret"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.register_user("bob", "hunter2"));
    approval_enabled = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.is_manual_login_approval_enabled(approval_enabled));
    FT_ASSERT_EQ(FT_FALSE, approval_enabled);
    approval_required = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.is_login_approval_required_for_user("alice", approval_required));
    FT_ASSERT_EQ(FT_FALSE, approval_required);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.set_login_approval_required_for_user("alice", FT_TRUE));
    approval_required = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.is_login_approval_required_for_user("alice", approval_required));
    FT_ASSERT_EQ(FT_TRUE, approval_required);
    authenticated = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_PERMISSION_DENIED, service.authenticate_user("alice", "s3cret", authenticated));
    FT_ASSERT_EQ(FT_FALSE, authenticated);
    authenticated = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.authenticate_user("bob", "hunter2", authenticated));
    FT_ASSERT_EQ(FT_TRUE, authenticated);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.approve_login("alice"));
    authenticated = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.authenticate_user("alice", "s3cret", authenticated));
    FT_ASSERT_EQ(FT_TRUE, authenticated);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.revoke_login_approval("alice"));
    authenticated = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_PERMISSION_DENIED, service.authenticate_user("alice", "s3cret", authenticated));
    FT_ASSERT_EQ(FT_FALSE, authenticated);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.set_manual_login_approval_enabled(FT_FALSE));
    approval_enabled = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.is_manual_login_approval_enabled(approval_enabled));
    FT_ASSERT_EQ(FT_FALSE, approval_enabled);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.set_login_approval_required_for_user("alice", FT_FALSE));
    approval_required = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.is_login_approval_required_for_user("alice", approval_required));
    FT_ASSERT_EQ(FT_FALSE, approval_required);
    FT_ASSERT_EQ(FT_ERR_ALREADY_EXISTS, service.register_user("alice", "s3cret"));
    authenticated = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.authenticate_user("alice", "s3cret", authenticated));
    FT_ASSERT_EQ(FT_TRUE, authenticated);
    authenticated = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_PERMISSION_DENIED, service.authenticate_user("alice", "wrong", authenticated));
    FT_ASSERT_EQ(FT_FALSE, authenticated);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.set_manual_login_approval_enabled(FT_TRUE));
    approval_enabled = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.is_manual_login_approval_enabled(approval_enabled));
    FT_ASSERT_EQ(FT_TRUE, approval_enabled);
    authenticated = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_PERMISSION_DENIED, service.authenticate_user("bob", "hunter2", authenticated));
    FT_ASSERT_EQ(FT_FALSE, authenticated);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.approve_login("bob"));
    authenticated = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.authenticate_user("bob", "hunter2", authenticated));
    FT_ASSERT_EQ(FT_TRUE, authenticated);
    exists = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.user_exists("alice", exists));
    FT_ASSERT_EQ(FT_TRUE, exists);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.remove_user("alice"));
    exists = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.user_exists("alice", exists));
    FT_ASSERT_EQ(FT_FALSE, exists);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, service.authenticate_user("alice", "s3cret", authenticated));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.destroy());
    cleanup_application_database(file_template);
    return (1);
}
