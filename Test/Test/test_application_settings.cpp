#include "../test_internal.hpp"
#include "../../Modules/Application/application.hpp"
#include "../../Modules/Filesystem/filesystem.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Time/time.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Storage/kv_store.hpp"
#include "../../Modules/Template/pair.hpp"
#include "../../Modules/Template/vector.hpp"

static int32_t create_application_database(const char *file_path)
{
    FILE *file_pointer;

    file_pointer = std::fopen(file_path, "w");
    if (file_pointer == ft_nullptr)
        return (FT_ERR_IO);
    if (std::fputs("{\n  \"kv_store\": {\n  }\n}\n", file_pointer) < 0)
    {
        (void)std::fclose(file_pointer);
        return (FT_ERR_IO);
    }
    if (std::fclose(file_pointer) != 0)
        return (FT_ERR_IO);
    return (FT_ERR_SUCCESS);
}

static int32_t cleanup_application_database(const char *file_path)
{
    return (std::remove(file_path));
}

static ft_bool parse_login_signal_output(const char *output_buffer, ft_string &token_output)
{
    const char *separator_position;
    ft_size_t token_length;

    if (output_buffer == ft_nullptr)
        return (FT_FALSE);
    separator_position = std::strstr(output_buffer, ": ");
    if (separator_position == ft_nullptr)
        return (FT_FALSE);
    separator_position += 2;
    while (*separator_position == ' ')
        separator_position++;
    token_output.destroy();
    if (token_output.initialize(separator_position) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    token_length = token_output.size();
    if (token_length > 0 && token_output.c_str()[token_length - 1] == '\n')
    {
        if (token_output.erase(token_length - 1, 1) != FT_ERR_SUCCESS)
            return (FT_FALSE);
    }
    return (FT_TRUE);
}

FT_TEST(test_application_settings_configuration_and_service_integration)
{
    application_settings settings;
    application_auth_service service;
    ft_string *database_file_path;
    ft_string root_path_output;
    ft_string relative_path_output;
    ft_string encryption_key_output;
    ft_string token_string;
    ft_bool enabled;
    ft_bool authenticated;
    char root_template[] = "/tmp/libft_application_root_XXXXXX";
    char database_relative_path[] = "auth_db.json";
    char *database_root_path;
    int pipe_descriptors[2];
    int32_t output_file_descriptor;
    int64_t timeout_seconds;
    char output_buffer[256];
    ssize_t read_result;

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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, create_application_database(database_file_path->c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.set_database_location(database_root_path, database_relative_path));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.set_encryption_key("app-auth-key-123"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.set_encryption_enabled(FT_TRUE));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.set_manual_login_approval_enabled(FT_FALSE));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.set_login_signal_token_timeout_seconds(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pipe(pipe_descriptors));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.set_login_signal_output_file_descriptor(pipe_descriptors[1]));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.get_database_root_path(root_path_output));
    FT_ASSERT_EQ(FT_TRUE, root_path_output == database_root_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.get_database_relative_path(relative_path_output));
    FT_ASSERT_EQ(FT_TRUE, relative_path_output == database_relative_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.get_encryption_key(encryption_key_output));
    FT_ASSERT_EQ(FT_TRUE, encryption_key_output == "app-auth-key-123");
    enabled = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.is_encryption_enabled(enabled));
    FT_ASSERT_EQ(FT_TRUE, enabled);
    enabled = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.is_manual_login_approval_enabled(enabled));
    FT_ASSERT_EQ(FT_FALSE, enabled);
    timeout_seconds = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.get_login_signal_token_timeout_seconds(timeout_seconds));
    FT_ASSERT_EQ(1, timeout_seconds);
    output_file_descriptor = -1;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.get_login_signal_output_file_descriptor(output_file_descriptor));
    FT_ASSERT_EQ(pipe_descriptors[1], output_file_descriptor);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.initialize(settings));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.register_user("alice", "s3cret"));
    timeout_seconds = 0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.get_login_signal_token_timeout_seconds(timeout_seconds));
    FT_ASSERT_EQ(1, timeout_seconds);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.get_login_signal_output_file_descriptor(output_file_descriptor));
    FT_ASSERT_EQ(pipe_descriptors[1], output_file_descriptor);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.request_login_signal_one_time_password("alice"));
    FT_ASSERT_EQ(0, close(pipe_descriptors[1]));
    std::memset(output_buffer, 0, sizeof(output_buffer));
    read_result = read(pipe_descriptors[0], output_buffer, sizeof(output_buffer) - 1);
    FT_ASSERT(read_result > 0);
    if (read_result > 0)
        output_buffer[static_cast<size_t>(read_result)] = '\0';
    FT_ASSERT_EQ(0, close(pipe_descriptors[0]));
    FT_ASSERT_EQ(FT_TRUE, parse_login_signal_output(output_buffer, token_string));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.authenticate_login_signal_one_time_password("alice", token_string.c_str(), authenticated));
    FT_ASSERT_EQ(FT_TRUE, authenticated);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, service.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, settings.destroy());
    FT_ASSERT_EQ(0, cleanup_application_database(database_file_path->c_str()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, database_file_path->destroy());
    delete database_file_path;
    FT_ASSERT_EQ(0, rmdir(database_root_path));
    return (1);
}
