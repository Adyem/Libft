#include "../../Encryption/encryption_hardware_acceleration.hpp"
#include "../../Encryption/aes.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

static bool g_encrypt_hook_invoked = false;
static bool g_decrypt_hook_invoked = false;

static void encryption_test_encrypt_hook(uint8_t *block, const uint8_t *key)
{
    g_encrypt_hook_invoked = true;
    aes_encrypt_software(block, key);
    return ;
}

static void encryption_test_decrypt_hook(uint8_t *block, const uint8_t *key)
{
    g_decrypt_hook_invoked = true;
    aes_decrypt_software(block, key);
    return ;
}

FT_TEST(test_encryption_hardware_hooks_override_aes,
    "Hardware acceleration hooks override AES implementation")
{
    uint8_t key[16];
    uint8_t plaintext[16];
    uint8_t expected_cipher[16];
    uint8_t cipher[16];
    uint8_t decrypted[16];
    uint8_t expected_plain[16];
    size_t index;
    s_encryption_hardware_hooks hooks;

    index = 0;
    while (index < 16)
    {
        key[index] = static_cast<uint8_t>(index);
        plaintext[index] = static_cast<uint8_t>(index * 3 + 1);
        expected_cipher[index] = plaintext[index];
        cipher[index] = plaintext[index];
        decrypted[index] = 0;
        index += 1;
    }
    aes_encrypt_software(expected_cipher, key);
    index = 0;
    while (index < 16)
    {
        expected_plain[index] = expected_cipher[index];
        index += 1;
    }
    aes_decrypt_software(expected_plain, key);

    encryption_clear_hardware_hooks();
    g_encrypt_hook_invoked = false;
    g_decrypt_hook_invoked = false;

    hooks.aes_encrypt = encryption_test_encrypt_hook;
    hooks.aes_decrypt = encryption_test_decrypt_hook;
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, encryption_register_hardware_hooks(hooks));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);

    aes_encrypt(cipher, key);
    FT_ASSERT_EQ(true, g_encrypt_hook_invoked);
    index = 0;
    while (index < 16)
    {
        FT_ASSERT_EQ(static_cast<int>(expected_cipher[index]), static_cast<int>(cipher[index]));
        index += 1;
    }

    index = 0;
    while (index < 16)
    {
        decrypted[index] = cipher[index];
        index += 1;
    }
    aes_decrypt(decrypted, key);
    FT_ASSERT_EQ(true, g_decrypt_hook_invoked);
    index = 0;
    while (index < 16)
    {
        FT_ASSERT_EQ(static_cast<int>(expected_plain[index]), static_cast<int>(decrypted[index]));
        index += 1;
    }

    encryption_clear_hardware_hooks();
    g_encrypt_hook_invoked = false;
    g_decrypt_hook_invoked = false;

    index = 0;
    while (index < 16)
    {
        cipher[index] = plaintext[index];
        index += 1;
    }
    aes_encrypt(cipher, key);
    FT_ASSERT_EQ(false, g_encrypt_hook_invoked);
    index = 0;
    while (index < 16)
    {
        FT_ASSERT_EQ(static_cast<int>(expected_cipher[index]), static_cast<int>(cipher[index]));
        index += 1;
    }
    return (1);
}

FT_TEST(test_encryption_hardware_hooks_query_state,
    "Hardware acceleration hook registry reports current state")
{
    s_encryption_hardware_hooks hooks;
    s_encryption_hardware_hooks queried;

    encryption_clear_hardware_hooks();
    hooks.aes_encrypt = encryption_test_encrypt_hook;
    hooks.aes_decrypt = encryption_test_decrypt_hook;
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, encryption_register_hardware_hooks(hooks));
    encryption_get_hardware_hooks(queried);
    FT_ASSERT_EQ(true, queried.aes_encrypt == encryption_test_encrypt_hook);
    FT_ASSERT_EQ(true, queried.aes_decrypt == encryption_test_decrypt_hook);

    encryption_clear_hardware_hooks();
    encryption_get_hardware_hooks(queried);
    FT_ASSERT_EQ(true, queried.aes_encrypt == ft_nullptr);
    FT_ASSERT_EQ(true, queried.aes_decrypt == ft_nullptr);
    return (1);
}
