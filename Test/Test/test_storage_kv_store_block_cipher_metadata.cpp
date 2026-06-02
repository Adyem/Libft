#include "../test_internal.hpp"
#include "../../Modules/Storage/kv_store.hpp"
#include "../../Modules/Encryption/encryption.hpp"
#include "../../Modules/File/file_utils.hpp"
#include "../../Modules/File/open_dir.hpp"
#include "../../Modules/Compatebility/compatebility_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include <cstdio>
#include <string>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#else
# include <unistd.h>
#endif

static int32_t test_storage_remove_directory_if_present(const char *directory_path)
{
    int directory_exists;
    int status;
    int error_code;

    directory_exists = 0;
    status = cmp_directory_exists(directory_path, &directory_exists, &error_code);
    if (status != FT_ERR_SUCCESS || directory_exists != 1)
        return (FT_ERR_SUCCESS);
#if defined(_WIN32) || defined(_WIN64)
    if (!RemoveDirectoryA(directory_path))
        return (FT_ERR_INVALID_OPERATION);
#else
    if (rmdir(directory_path) != 0)
        return (FT_ERR_INVALID_OPERATION);
#endif
    return (FT_ERR_SUCCESS);
}

static int32_t test_storage_cleanup_paths(const char *directory_path, const char *file_path)
{
    int32_t delete_error;
    int32_t directory_error;

    delete_error = file_delete(file_path);
    if (delete_error != FT_ERR_SUCCESS
        && delete_error != FT_ERR_NOT_FOUND
        && delete_error != FT_ERR_INVALID_OPERATION)
        return (delete_error);
    directory_error = test_storage_remove_directory_if_present(directory_path);
    if (directory_error != FT_ERR_SUCCESS)
        return (directory_error);
    return (FT_ERR_SUCCESS);
}

static int32_t test_storage_create_kv_store_file(const char *file_path)
{
    FILE *file_pointer;

    file_pointer = ft_fopen(file_path, "w");
    if (file_pointer == ft_nullptr)
        return (FT_ERR_INVALID_OPERATION);
    if (std::fputs("{\n  \"kv_store\": {\n    \"__placeholder__\": \"\"\n  }\n}\n", file_pointer) < 0)
    {
        ft_fclose(file_pointer);
        return (FT_ERR_INVALID_OPERATION);
    }
    if (ft_fclose(file_pointer) != 0)
        return (FT_ERR_INVALID_OPERATION);
    return (FT_ERR_SUCCESS);
}

static std::string test_storage_read_file_contents(const char *file_path)
{
    FILE *file_pointer;
    long file_size;
    std::string file_content;
    std::size_t bytes_read;

    file_pointer = ft_fopen(file_path, "rb");
    if (file_pointer == ft_nullptr)
        return (std::string());
    if (std::fseek(file_pointer, 0, SEEK_END) != 0)
    {
        ft_fclose(file_pointer);
        return (std::string());
    }
    file_size = std::ftell(file_pointer);
    if (file_size < 0)
    {
        ft_fclose(file_pointer);
        return (std::string());
    }
    if (std::fseek(file_pointer, 0, SEEK_SET) != 0)
    {
        ft_fclose(file_pointer);
        return (std::string());
    }
    file_content.assign(static_cast<std::size_t>(file_size), '\0');
    if (file_size == 0)
    {
        ft_fclose(file_pointer);
        return (file_content);
    }
    bytes_read = std::fread(&file_content[0], 1, file_content.size(), file_pointer);
    ft_fclose(file_pointer);
    if (bytes_read != file_content.size())
        return (std::string());
    return (file_content);
}

static void test_storage_block_cipher_encrypt(uint8_t *block_buffer, const uint8_t *key_buffer)
{
    aes_encrypt_software(block_buffer, key_buffer);
    return ;
}

static void test_storage_block_cipher_decrypt(uint8_t *block_buffer, const uint8_t *key_buffer)
{
    aes_decrypt_software(block_buffer, key_buffer);
    return ;
}

FT_TEST(test_kv_store_encrypted_round_trip_uses_active_algorithm_name)
{
    char directory_template[] = "/tmp/libft_storage_block_cipher_XXXXXX";
    char file_path_buffer[256];
    const char *directory_path;
    const char *file_path;
    const char *encryption_key;
    const char *encryption_algorithm_name;
    s_encryption_block_hooks block_hooks;
    kv_store encrypted_store;
    kv_store reloaded_store;
    std::string file_content;
    int32_t cleanup_error;

#if defined(_WIN32) || defined(_WIN64)
    directory_path = "C:/Temp/libft_storage_block_cipher";
    file_path = "C:/Temp/libft_storage_block_cipher/kv_store.json";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, test_storage_cleanup_paths(directory_path, file_path));
    FT_ASSERT_EQ(0, file_create_directory(directory_path, 0700));
#else
    directory_path = mkdtemp(directory_template);
    FT_ASSERT(directory_path != ft_nullptr);
#endif
#if !defined(_WIN32) && !defined(_WIN64)
    std::snprintf(file_path_buffer, sizeof(file_path_buffer),
        "%s/kv_store.json", directory_path);
    file_path = file_path_buffer;
#endif
    encryption_key = "sixteen-byte-key";
    encryption_algorithm_name = "unit-test-block-cipher";
    FT_ASSERT_EQ(FT_ERR_SUCCESS, test_storage_create_kv_store_file(file_path));

    encryption_clear_block_hooks();
    block_hooks.encrypt = test_storage_block_cipher_encrypt;
    block_hooks.decrypt = test_storage_block_cipher_decrypt;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, encryption_register_block_hooks(block_hooks, "unit-test-block-cipher"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, encrypted_store.initialize(file_path, encryption_key, FT_TRUE, encryption_algorithm_name));
    FT_ASSERT_EQ(0, encrypted_store.kv_delete("__placeholder__"));
    FT_ASSERT_EQ(0, encrypted_store.kv_set("secret", "value"));
    FT_ASSERT_EQ(0, encrypted_store.kv_flush());

    file_content = test_storage_read_file_contents(file_path);
    FT_ASSERT(file_content.find("unit-test-block-cipher") != std::string::npos);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, reloaded_store.initialize(file_path, encryption_key, FT_TRUE, encryption_algorithm_name));
    FT_ASSERT(reloaded_store.kv_get("secret") != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(reloaded_store.kv_get("secret"), "value"));

    FT_ASSERT_EQ(FT_ERR_SUCCESS, reloaded_store.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, encrypted_store.destroy());
    encryption_clear_block_hooks();
    cleanup_error = test_storage_cleanup_paths(directory_path, file_path);
    FT_ASSERT(cleanup_error == FT_ERR_SUCCESS
        || cleanup_error == FT_ERR_INVALID_OPERATION);
    return (1);
}
