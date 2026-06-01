#include "../test_internal.hpp"
#include "../../Modules/Encryption/encryption.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

static ft_bool g_first_encrypt_invoked = FT_FALSE;
static ft_bool g_first_decrypt_invoked = FT_FALSE;
static ft_bool g_second_encrypt_invoked = FT_FALSE;
static ft_bool g_second_decrypt_invoked = FT_FALSE;

static void encryption_test_first_encrypt_hook(uint8_t *block_buffer, const uint8_t *key_buffer)
{
    ft_size_t index;

    g_first_encrypt_invoked = FT_TRUE;
    index = 0;
    while (index < 16U)
    {
        block_buffer[index] ^= static_cast<uint8_t>(0x11U);
        block_buffer[index] ^= key_buffer[index];
        index++;
    }
    return ;
}

static void encryption_test_first_decrypt_hook(uint8_t *block_buffer, const uint8_t *key_buffer)
{
    ft_size_t index;

    g_first_decrypt_invoked = FT_TRUE;
    index = 0;
    while (index < 16U)
    {
        block_buffer[index] ^= key_buffer[index];
        block_buffer[index] ^= static_cast<uint8_t>(0x11U);
        index++;
    }
    return ;
}

static void encryption_test_second_encrypt_hook(uint8_t *block_buffer, const uint8_t *key_buffer)
{
    ft_size_t index;

    g_second_encrypt_invoked = FT_TRUE;
    index = 0;
    while (index < 16U)
    {
        block_buffer[index] ^= static_cast<uint8_t>(0x55U);
        block_buffer[index] ^= key_buffer[index];
        index++;
    }
    return ;
}

static void encryption_test_second_decrypt_hook(uint8_t *block_buffer, const uint8_t *key_buffer)
{
    ft_size_t index;

    g_second_decrypt_invoked = FT_TRUE;
    index = 0;
    while (index < 16U)
    {
        block_buffer[index] ^= key_buffer[index];
        block_buffer[index] ^= static_cast<uint8_t>(0x55U);
        index++;
    }
    return ;
}

FT_TEST(test_encryption_block_hooks_can_be_swapped_at_runtime)
{
    uint8_t key_buffer[16];
    uint8_t plaintext_buffer[16];
    uint8_t cipher_buffer[16];
    uint8_t expected_first_cipher[16];
    uint8_t expected_second_cipher[16];
    uint8_t expected_software_cipher[16];
    s_encryption_block_hooks block_hooks;
    s_encryption_block_hooks queried_hooks;
    ft_string algorithm_name;
    ft_size_t index;

    encryption_clear_block_hooks();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, algorithm_name.initialize());
    index = 0;
    while (index < 16U)
    {
        key_buffer[index] = static_cast<uint8_t>(index);
        plaintext_buffer[index] = static_cast<uint8_t>(index * 3U + 1U);
        expected_first_cipher[index] = plaintext_buffer[index];
        expected_second_cipher[index] = plaintext_buffer[index];
        expected_software_cipher[index] = plaintext_buffer[index];
        cipher_buffer[index] = plaintext_buffer[index];
        index++;
    }

    block_hooks.encrypt = encryption_test_first_encrypt_hook;
    block_hooks.decrypt = encryption_test_first_decrypt_hook;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, encryption_register_block_hooks(block_hooks, "first-block-cipher"));
    encryption_get_block_hooks(queried_hooks);
    FT_ASSERT_EQ(true, queried_hooks.encrypt == encryption_test_first_encrypt_hook);
    FT_ASSERT_EQ(true, queried_hooks.decrypt == encryption_test_first_decrypt_hook);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, encryption_get_block_cipher_name(algorithm_name));
    FT_ASSERT_EQ(0, ft_strcmp(algorithm_name.c_str(), "first-block-cipher"));

    aes_encrypt(expected_first_cipher, key_buffer);
    FT_ASSERT_EQ(FT_TRUE, g_first_encrypt_invoked);
    FT_ASSERT_EQ(FT_FALSE, g_second_encrypt_invoked);
    aes_decrypt(expected_first_cipher, key_buffer);
    FT_ASSERT_EQ(FT_TRUE, g_first_decrypt_invoked);
    index = 0;
    while (index < 16U)
    {
        FT_ASSERT_EQ(static_cast<int>(plaintext_buffer[index]), static_cast<int>(expected_first_cipher[index]));
        index++;
    }

    g_first_encrypt_invoked = FT_FALSE;
    g_first_decrypt_invoked = FT_FALSE;
    g_second_encrypt_invoked = FT_FALSE;
    g_second_decrypt_invoked = FT_FALSE;

    index = 0;
    while (index < 16U)
    {
        cipher_buffer[index] = plaintext_buffer[index];
        index++;
    }

    block_hooks.encrypt = encryption_test_second_encrypt_hook;
    block_hooks.decrypt = encryption_test_second_decrypt_hook;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, encryption_register_block_hooks(block_hooks, "second-block-cipher"));
    encryption_get_block_hooks(queried_hooks);
    FT_ASSERT_EQ(true, queried_hooks.encrypt == encryption_test_second_encrypt_hook);
    FT_ASSERT_EQ(true, queried_hooks.decrypt == encryption_test_second_decrypt_hook);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, encryption_get_block_cipher_name(algorithm_name));
    FT_ASSERT_EQ(0, ft_strcmp(algorithm_name.c_str(), "second-block-cipher"));

    aes_encrypt(cipher_buffer, key_buffer);
    FT_ASSERT_EQ(FT_FALSE, g_first_encrypt_invoked);
    FT_ASSERT_EQ(FT_TRUE, g_second_encrypt_invoked);
    index = 0;
    while (index < 16U)
    {
        expected_second_cipher[index] ^= static_cast<uint8_t>(0x55U);
        expected_second_cipher[index] ^= key_buffer[index];
        FT_ASSERT_EQ(static_cast<int>(expected_second_cipher[index]), static_cast<int>(cipher_buffer[index]));
        index++;
    }
    aes_decrypt(cipher_buffer, key_buffer);
    FT_ASSERT_EQ(FT_TRUE, g_second_decrypt_invoked);
    index = 0;
    while (index < 16U)
    {
        FT_ASSERT_EQ(static_cast<int>(plaintext_buffer[index]), static_cast<int>(cipher_buffer[index]));
        index++;
    }

    encryption_clear_block_hooks();
    g_second_encrypt_invoked = FT_FALSE;
    g_second_decrypt_invoked = FT_FALSE;
    index = 0;
    while (index < 16U)
    {
        cipher_buffer[index] = plaintext_buffer[index];
        expected_second_cipher[index] = plaintext_buffer[index];
        expected_software_cipher[index] = plaintext_buffer[index];
        index++;
    }
    aes_encrypt(cipher_buffer, key_buffer);
    aes_encrypt_software(expected_software_cipher, key_buffer);
    index = 0;
    while (index < 16U)
    {
        FT_ASSERT_EQ(static_cast<int>(expected_software_cipher[index]), static_cast<int>(cipher_buffer[index]));
        index++;
    }
    FT_ASSERT_EQ(FT_FALSE, g_second_encrypt_invoked);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, encryption_get_block_cipher_name(algorithm_name));
    FT_ASSERT_EQ(0, ft_strcmp(algorithm_name.c_str(), "aes-128-ecb-base64"));
    return (1);
}
