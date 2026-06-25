#include "encryption.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_string.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

#include <atomic>
#include <string>
#include "../Basic/limits.hpp"
#include "../PThread/recursive_mutex.hpp"

static std::atomic<t_encryption_block_function> g_block_encrypt_hook;
static std::atomic<t_encryption_block_function> g_block_decrypt_hook;
static pt_mutex g_block_cipher_name_mutex;
static std::atomic<uint8_t> g_block_cipher_name_mutex_state(
    static_cast<uint8_t>(0U));
static std::string g_block_cipher_name;

static const char g_default_block_cipher_name[] = "aes-128-ecb-base64";
static const uint8_t g_block_cipher_name_mutex_state_uninitialised = static_cast<uint8_t>(0U);
static const uint8_t g_block_cipher_name_mutex_state_initialising = static_cast<uint8_t>(1U);
static const uint8_t g_block_cipher_name_mutex_state_initialised = static_cast<uint8_t>(2U);

static int32_t encryption_ensure_block_cipher_name_mutex(void)
{
    uint8_t expected_state;
    uint8_t current_state;

    for (;;)
    {
        current_state = g_block_cipher_name_mutex_state.load(std::memory_order_acquire);
        if (current_state == g_block_cipher_name_mutex_state_initialised)
            return (FT_ERR_SUCCESS);
        if (current_state == g_block_cipher_name_mutex_state_uninitialised)
        {
            expected_state = g_block_cipher_name_mutex_state_uninitialised;
            if (g_block_cipher_name_mutex_state.compare_exchange_strong(
                    expected_state,
                    g_block_cipher_name_mutex_state_initialising,
                    std::memory_order_acq_rel,
                    std::memory_order_acquire))
            {
                int32_t initialize_error;

                initialize_error = g_block_cipher_name_mutex.initialize();
                if (initialize_error != FT_ERR_SUCCESS)
                {
                    g_block_cipher_name_mutex_state.store(
                        g_block_cipher_name_mutex_state_uninitialised,
                        std::memory_order_release);
                    return (initialize_error);
                }
                g_block_cipher_name_mutex_state.store(
                    g_block_cipher_name_mutex_state_initialised,
                    std::memory_order_release);
                return (FT_ERR_SUCCESS);
            }
            continue ;
        }
        (void)pt_thread_yield();
    }
}

static int32_t encryption_reset_block_cipher_name(void)
{
    int32_t mutex_error;

    mutex_error = encryption_ensure_block_cipher_name_mutex();
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    mutex_error = g_block_cipher_name_mutex.lock();
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    g_block_cipher_name.clear();
    (void)g_block_cipher_name_mutex.unlock();
    return (FT_ERR_SUCCESS);
}

static int32_t encryption_store_block_cipher_name(const char *algorithm_name)
{
    int32_t mutex_error;

    if (algorithm_name == ft_nullptr)
        algorithm_name = g_default_block_cipher_name;
    mutex_error = encryption_ensure_block_cipher_name_mutex();
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    mutex_error = g_block_cipher_name_mutex.lock();
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    g_block_cipher_name = algorithm_name;
    (void)g_block_cipher_name_mutex.unlock();
    return (FT_ERR_SUCCESS);
}

int32_t encryption_register_block_hooks(const s_encryption_block_hooks &hooks,
    const char *algorithm_name)
{
    int32_t name_error;

    if (algorithm_name == ft_nullptr)
        algorithm_name = g_default_block_cipher_name;
    name_error = encryption_store_block_cipher_name(algorithm_name);
    if (name_error != FT_ERR_SUCCESS)
        return (name_error);
    g_block_encrypt_hook.store(hooks.encrypt, std::memory_order_release);
    g_block_decrypt_hook.store(hooks.decrypt, std::memory_order_release);
    return (FT_ERR_SUCCESS);
}

void encryption_clear_block_hooks(void)
{
    g_block_encrypt_hook.store(ft_nullptr, std::memory_order_release);
    g_block_decrypt_hook.store(ft_nullptr, std::memory_order_release);
    (void)encryption_reset_block_cipher_name();
    return ;
}

void encryption_get_block_hooks(s_encryption_block_hooks &out_hooks)
{
    out_hooks.encrypt = g_block_encrypt_hook.load(std::memory_order_acquire);
    out_hooks.decrypt = g_block_decrypt_hook.load(std::memory_order_acquire);
    return ;
}

int32_t encryption_get_block_cipher_name(ft_string &algorithm_name_output)
{
    const char *cipher_name;
    int32_t mutex_error;

    mutex_error = encryption_ensure_block_cipher_name_mutex();
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    mutex_error = g_block_cipher_name_mutex.lock();
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    if (g_block_cipher_name.empty())
        cipher_name = g_default_block_cipher_name;
    else
        cipher_name = g_block_cipher_name.c_str();
    (void)g_block_cipher_name_mutex.unlock();
    if (cipher_name == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    algorithm_name_output = cipher_name;
    if (algorithm_name_output.c_str() == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    return (FT_ERR_SUCCESS);
}

ft_bool encryption_try_block_encrypt(uint8_t *block,
    const uint8_t *key)
{
    t_encryption_block_function hook;

    hook = g_block_encrypt_hook.load(std::memory_order_acquire);
    if (hook == ft_nullptr)
        return (FT_FALSE);
    hook(block, key);
    return (FT_TRUE);
}

ft_bool encryption_try_block_decrypt(uint8_t *block,
    const uint8_t *key)
{
    t_encryption_block_function hook;

    hook = g_block_decrypt_hook.load(std::memory_order_acquire);
    if (hook == ft_nullptr)
        return (FT_FALSE);
    hook(block, key);
    return (FT_TRUE);
}

int32_t encryption_register_hardware_hooks(
    const s_encryption_hardware_hooks &hooks,
    const char *algorithm_name)
{
    return (encryption_register_block_hooks(hooks, algorithm_name));
}

void encryption_clear_hardware_hooks(void)
{
    encryption_clear_block_hooks();
    return ;
}

void encryption_get_hardware_hooks(s_encryption_hardware_hooks &out_hooks)
{
    encryption_get_block_hooks(out_hooks);
    return ;
}

ft_bool encryption_try_hardware_aes_encrypt(uint8_t *block,
    const uint8_t *key)
{
    return (encryption_try_block_encrypt(block, key));
}

ft_bool encryption_try_hardware_aes_decrypt(uint8_t *block,
    const uint8_t *key)
{
    return (encryption_try_block_decrypt(block, key));
}
