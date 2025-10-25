#include "encryption_hardware_acceleration.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

#include <atomic>

static std::atomic<t_encryption_aes_block_function> g_aes_encrypt_hook;
static std::atomic<t_encryption_aes_block_function> g_aes_decrypt_hook;

int encryption_register_hardware_hooks(const s_encryption_hardware_hooks &hooks)
{
    g_aes_encrypt_hook.store(hooks.aes_encrypt, std::memory_order_release);
    g_aes_decrypt_hook.store(hooks.aes_decrypt, std::memory_order_release);
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

void encryption_clear_hardware_hooks(void)
{
    g_aes_encrypt_hook.store(ft_nullptr, std::memory_order_release);
    g_aes_decrypt_hook.store(ft_nullptr, std::memory_order_release);
    ft_errno = ER_SUCCESS;
    return ;
}

void encryption_get_hardware_hooks(s_encryption_hardware_hooks &out_hooks)
{
    out_hooks.aes_encrypt = g_aes_encrypt_hook.load(std::memory_order_acquire);
    out_hooks.aes_decrypt = g_aes_decrypt_hook.load(std::memory_order_acquire);
    ft_errno = ER_SUCCESS;
    return ;
}

bool encryption_try_hardware_aes_encrypt(uint8_t *block, const uint8_t *key)
{
    t_encryption_aes_block_function hook;

    hook = g_aes_encrypt_hook.load(std::memory_order_acquire);
    if (hook == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (false);
    }
    hook(block, key);
    ft_errno = ER_SUCCESS;
    return (true);
}

bool encryption_try_hardware_aes_decrypt(uint8_t *block, const uint8_t *key)
{
    t_encryption_aes_block_function hook;

    hook = g_aes_decrypt_hook.load(std::memory_order_acquire);
    if (hook == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (false);
    }
    hook(block, key);
    ft_errno = ER_SUCCESS;
    return (true);
}
