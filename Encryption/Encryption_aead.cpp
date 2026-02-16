#include "encryption_aead.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>
#include <cstdio>

#if NETWORKING_HAS_OPENSSL

static const EVP_CIPHER *encryption_aead_select_cipher(size_t key_length)
{
    if (key_length == 16)
        return (EVP_aes_128_gcm());
    if (key_length == 24)
        return (EVP_aes_192_gcm());
    if (key_length == 32)
        return (EVP_aes_256_gcm());
    return (NULL);
}

encryption_aead_context::encryption_aead_context()
{
    this->_context = NULL;
    this->_cipher = NULL;
    this->_encrypt_mode = false;
    this->_initialized = false;
    this->_iv_length = 0;
    this->_mutex = ft_nullptr;
    this->_initialized_state = this->_state_uninitialized;
    return ;
}

encryption_aead_context::~encryption_aead_context()
{
    if (this->_initialized_state == this->_state_initialized)
        (void)this->destroy();
    else if (this->_initialized_state == this->_state_uninitialized)
        this->abort_lifecycle_error("~encryption_aead_context",
            "destroyed while uninitialized");
    return ;
}

void encryption_aead_context::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    ft_fprintf(stderr, "encryption_aead_context::%s lifecycle error: %s\n",
        method_name, reason);
    su_abort();
}

void encryption_aead_context::abort_if_not_initialized(
    const char *method_name) const
{
    if (this->_initialized_state != this->_state_initialized)
        this->abort_lifecycle_error(method_name, "object is not initialized");
}

int encryption_aead_context::enable_thread_safety()
{
    pt_recursive_mutex *new_mutex;
    int mutex_error;

    this->abort_if_not_initialized("enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = new_mutex->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        return (mutex_error);
    }
    this->_mutex = new_mutex;
    return (FT_ERR_SUCCESS);
}

int encryption_aead_context::disable_thread_safety()
{
    int destroy_error;

    this->abort_if_not_initialized("disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool encryption_aead_context::is_thread_safe() const
{
    this->abort_if_not_initialized("is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int encryption_aead_context::initialize()
{
    int mutex_error;

    if (this->_initialized_state == this->_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_initialized_state = this->_state_initialized;
    mutex_error = this->enable_thread_safety();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = this->_state_destroyed;
        return (mutex_error);
    }
    this->_context = EVP_CIPHER_CTX_new();
    if (this->_context == NULL)
    {
        (void)this->disable_thread_safety();
        this->_initialized_state = this->_state_destroyed;
        return (FT_ERR_NO_MEMORY);
    }
    return (FT_ERR_SUCCESS);
}

int encryption_aead_context::destroy()
{
    int disable_error;

    if (this->_initialized_state != this->_state_initialized)
        return (FT_ERR_INVALID_STATE);
    if (this->_context != NULL)
    {
        EVP_CIPHER_CTX_free(this->_context);
        this->_context = NULL;
    }
    disable_error = this->disable_thread_safety();
    this->_cipher = NULL;
    this->_encrypt_mode = false;
    this->_initialized = false;
    this->_iv_length = 0;
    this->_initialized_state = this->_state_destroyed;
    return (disable_error);
}

int encryption_aead_context::finalize_operation(int result) const
{
    this->abort_if_not_initialized("finalize_operation");
    int unlock_error;

    if (this->_mutex == ft_nullptr)
        unlock_error = FT_ERR_SUCCESS;
    else
        unlock_error = this->_mutex->unlock();
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (result);
}

int encryption_aead_context::configure_cipher(const unsigned char *key, size_t key_length,
        const unsigned char *iv, size_t iv_length, bool encrypt_mode)
{
    this->abort_if_not_initialized("configure_cipher");
    const EVP_CIPHER *cipher;
    int control_result;
    int iv_length_value;
    int encrypt_flag = 0;
    int lock_result;

    if (this->_mutex == ft_nullptr)
        lock_result = FT_ERR_SUCCESS;
    else
        lock_result = this->_mutex->lock();

    if (lock_result != FT_ERR_SUCCESS)
        return (lock_result);
    if (this->_context == NULL)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    cipher = encryption_aead_select_cipher(key_length);
    if (cipher == NULL)
        return (this->finalize_operation(FT_ERR_UNSUPPORTED_TYPE));
    if (EVP_CIPHER_CTX_reset(this->_context) != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    if (encrypt_mode)
        encrypt_flag = 1;
    if (EVP_CipherInit_ex(this->_context, cipher, NULL, NULL, NULL,
            encrypt_flag) != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    iv_length_value = EVP_CIPHER_iv_length(cipher);
    if (iv_length_value < 0)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    this->_iv_length = static_cast<size_t>(iv_length_value);
    if (iv != NULL && iv_length != this->_iv_length)
    {
        control_result = EVP_CIPHER_CTX_ctrl(this->_context,
                EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(iv_length), NULL);
        if (control_result != 1)
            return (this->finalize_operation(FT_ERR_INVALID_ARGUMENT));
        this->_iv_length = iv_length;
    }
    if (EVP_CipherInit_ex(this->_context, NULL, NULL, key, iv, -1) != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    this->_cipher = cipher;
    this->_encrypt_mode = encrypt_mode;
    this->_initialized = true;
    return (this->finalize_operation(FT_ERR_SUCCESS));
}

int encryption_aead_context::initialize_encrypt(const unsigned char *key, size_t key_length,
        const unsigned char *iv, size_t iv_length)
{
    this->abort_if_not_initialized("initialize_encrypt");
    return (this->configure_cipher(key, key_length, iv, iv_length, true));
}

int encryption_aead_context::initialize_decrypt(const unsigned char *key, size_t key_length,
        const unsigned char *iv, size_t iv_length)
{
    this->abort_if_not_initialized("initialize_decrypt");
    return (this->configure_cipher(key, key_length, iv, iv_length, false));
}

int encryption_aead_context::update_aad(const unsigned char *aad, size_t aad_length)
{
    this->abort_if_not_initialized("update_aad");
    int lock_result;

    if (this->_mutex == ft_nullptr)
        lock_result = FT_ERR_SUCCESS;
    else
        lock_result = this->_mutex->lock();

    if (lock_result != FT_ERR_SUCCESS)
        return (lock_result);
    if (!this->_initialized)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    if (aad_length == 0)
        return (this->finalize_operation(FT_ERR_SUCCESS));
    int update_result;
    int out_length = 0;

    update_result = EVP_CipherUpdate(this->_context, NULL, &out_length,
            aad, static_cast<int>(aad_length));
    if (update_result != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    return (this->finalize_operation(FT_ERR_SUCCESS));
}

int encryption_aead_context::update(const unsigned char *input, size_t input_length,
        unsigned char *output, size_t &output_length)
{
    this->abort_if_not_initialized("update");
    int lock_result;

    if (this->_mutex == ft_nullptr)
        lock_result = FT_ERR_SUCCESS;
    else
        lock_result = this->_mutex->lock();

    output_length = 0;
    if (lock_result != FT_ERR_SUCCESS)
        return (lock_result);
    if (!this->_initialized)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    if (input_length == 0)
        return (this->finalize_operation(FT_ERR_SUCCESS));
    int local_length = 0;
    int update_result = EVP_CipherUpdate(this->_context, output, &local_length,
            input, static_cast<int>(input_length));
    if (update_result != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    if (local_length < 0)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    output_length = static_cast<size_t>(local_length);
    return (this->finalize_operation(FT_ERR_SUCCESS));
}

int encryption_aead_context::finalize(unsigned char *tag, size_t tag_length)
{
    this->abort_if_not_initialized("finalize");
    int lock_result;

    if (this->_mutex == ft_nullptr)
        lock_result = FT_ERR_SUCCESS;
    else
        lock_result = this->_mutex->lock();

    if (lock_result != FT_ERR_SUCCESS)
        return (lock_result);
    if (!this->_initialized)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    int final_result = 0;
    unsigned char final_block[EVP_MAX_BLOCK_LENGTH];
    int final_length = 0;

    ft_memset(final_block, 0, sizeof(final_block));
    final_result = EVP_CipherFinal_ex(this->_context, final_block, &final_length);
    if (final_result != 1)
    {
        if (this->_encrypt_mode)
            return (this->finalize_operation(FT_ERR_INTERNAL));
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    }
    if (final_length < 0)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    if (final_length != 0)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    if (!this->_encrypt_mode)
    {
        this->_initialized = false;
        return (this->finalize_operation(FT_ERR_SUCCESS));
    }
    if (tag == NULL)
        return (this->finalize_operation(FT_ERR_INVALID_ARGUMENT));
    if (tag_length == 0 || tag_length > 16)
        return (this->finalize_operation(FT_ERR_INVALID_ARGUMENT));
    if (EVP_CIPHER_CTX_ctrl(this->_context, EVP_CTRL_GCM_GET_TAG,
            static_cast<int>(tag_length), tag) != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    this->_initialized = false;
    return (this->finalize_operation(FT_ERR_SUCCESS));
}

int encryption_aead_context::set_tag(const unsigned char *tag, size_t tag_length)
{
    this->abort_if_not_initialized("set_tag");
    int lock_result;

    if (this->_mutex == ft_nullptr)
        lock_result = FT_ERR_SUCCESS;
    else
        lock_result = this->_mutex->lock();

    if (lock_result != FT_ERR_SUCCESS)
        return (lock_result);
    if (!this->_initialized)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    if (this->_encrypt_mode)
        return (this->finalize_operation(FT_ERR_INVALID_OPERATION));
    if (tag == NULL)
        return (this->finalize_operation(FT_ERR_INVALID_ARGUMENT));
    if (tag_length == 0 || tag_length > 16)
        return (this->finalize_operation(FT_ERR_INVALID_ARGUMENT));
    if (EVP_CIPHER_CTX_ctrl(this->_context, EVP_CTRL_GCM_SET_TAG,
            static_cast<int>(tag_length), const_cast<unsigned char *>(tag)) != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    return (this->finalize_operation(FT_ERR_SUCCESS));
}

void    encryption_aead_context::reset()
{
    this->abort_if_not_initialized("reset");
    int lock_result;

    if (this->_mutex == ft_nullptr)
        lock_result = FT_ERR_SUCCESS;
    else
        lock_result = this->_mutex->lock();
    if (lock_result != FT_ERR_SUCCESS)
        return ;
    if (this->_context != NULL)
        EVP_CIPHER_CTX_reset(this->_context);
    this->_cipher = NULL;
    this->_encrypt_mode = false;
    this->_initialized = false;
    this->_iv_length = 0;
    int unlock_result;

    if (this->_mutex == ft_nullptr)
        unlock_result = FT_ERR_SUCCESS;
    else
        unlock_result = this->_mutex->unlock();
    if (unlock_result != FT_ERR_SUCCESS)
        return ;
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *encryption_aead_context::get_mutex_for_validation() const
{
    return (this->_mutex);
}
#endif

bool    encryption_aead_encrypt(const unsigned char *key, size_t key_length,
        const unsigned char *iv, size_t iv_length,
        const unsigned char *aad, size_t aad_length,
        const unsigned char *plaintext, size_t plaintext_length,
        unsigned char *ciphertext, unsigned char *tag, size_t tag_length)
{
    encryption_aead_context context;
    size_t output_length;

    if (context.initialize() != FT_ERR_SUCCESS)
        return (false);
    if (context.initialize_encrypt(key, key_length, iv, iv_length) != FT_ERR_SUCCESS)
        return (false);
    if (context.update_aad(aad, aad_length) != FT_ERR_SUCCESS)
        return (false);
    output_length = 0;
    if (plaintext_length > 0)
    {
        if (ciphertext == NULL)
            return (false);
        if (context.update(plaintext, plaintext_length, ciphertext,
                output_length) != FT_ERR_SUCCESS)
            return (false);
        if (output_length != plaintext_length)
            return (false);
    }
    if (context.finalize(tag, tag_length) != FT_ERR_SUCCESS)
        return (false);
    return (true);
}

bool    encryption_aead_decrypt(const unsigned char *key, size_t key_length,
            const unsigned char *iv, size_t iv_length,
            const unsigned char *aad, size_t aad_length,
            const unsigned char *ciphertext, size_t ciphertext_length,
            const unsigned char *tag, size_t tag_length,
            unsigned char *plaintext)
{
    encryption_aead_context context;
    size_t output_length;

    if (context.initialize() != FT_ERR_SUCCESS)
        return (false);
    if (context.initialize_decrypt(key, key_length, iv, iv_length) != FT_ERR_SUCCESS)
        return (false);
    if (context.set_tag(tag, tag_length) != FT_ERR_SUCCESS)
        return (false);
    if (context.update_aad(aad, aad_length) != FT_ERR_SUCCESS)
        return (false);
    output_length = 0;
    if (ciphertext_length > 0)
    {
        if (plaintext == NULL)
            return (false);
        if (context.update(ciphertext, ciphertext_length, plaintext,
                output_length) != FT_ERR_SUCCESS)
            return (false);
        if (output_length != ciphertext_length)
            return (false);
    }
    if (context.finalize(NULL, 0) != FT_ERR_SUCCESS)
        return (false);
    return (true);
}

#endif // NETWORKING_HAS_OPENSSL
