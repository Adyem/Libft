#include "encryption_aead.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread_internal.hpp"

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

static int encryption_aead_initialize_mutex(pt_recursive_mutex **mutex_pointer)
{
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    *mutex_pointer = ft_nullptr;
    return (pt_recursive_mutex_create_with_error(mutex_pointer));
}

static void encryption_aead_destroy_mutex(pt_recursive_mutex **mutex_pointer)
{
    pt_recursive_mutex_destroy(mutex_pointer);
}

encryption_aead_context::encryption_aead_context()
{
    this->_context = EVP_CIPHER_CTX_new();
    this->_cipher = NULL;
    this->_encrypt_mode = false;
    this->_initialized = false;
    this->_iv_length = 0;
    this->_mutex = ft_nullptr;
    int mutex_error = encryption_aead_initialize_mutex(&this->_mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(mutex_error);
        return ;
    }
    if (this->_context == NULL)
    {
        encryption_aead_destroy_mutex(&this->_mutex);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return ;
    }
    return ;
}

encryption_aead_context::~encryption_aead_context()
{
    if (this->_context != NULL)
    {
        EVP_CIPHER_CTX_free(this->_context);
        this->_context = NULL;
    }
    encryption_aead_destroy_mutex(&this->_mutex);
    return ;
}

int encryption_aead_context::finalize_operation(int result) const
{
    int unlock_error = pt_recursive_mutex_unlock_if_valid(this->_mutex);
    ft_global_error_stack_drop_last_error();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(result);
    return (result);
}

encryption_aead_context::encryption_aead_context(encryption_aead_context &&other) noexcept
{
    this->_context = NULL;
    this->_cipher = NULL;
    this->_encrypt_mode = false;
    this->_initialized = false;
    this->_iv_length = 0;
    this->_mutex = ft_nullptr;
    encryption_aead_initialize_mutex(&this->_mutex);
    int self_lock = pt_recursive_mutex_lock_if_valid(this->_mutex);
    ft_global_error_stack_drop_last_error();
    if (self_lock != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(self_lock);
        return ;
    }
    int other_lock = pt_recursive_mutex_lock_if_valid(other._mutex);
    ft_global_error_stack_drop_last_error();
    if (other_lock != FT_ERR_SUCCESSS)
    {
        pt_recursive_mutex_unlock_if_valid(this->_mutex);
        ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(other_lock);
        return ;
    }
    this->_context = other._context;
    this->_cipher = other._cipher;
    this->_encrypt_mode = other._encrypt_mode;
    this->_initialized = other._initialized;
    this->_iv_length = other._iv_length;
    other._context = NULL;
    other._cipher = NULL;
    other._encrypt_mode = false;
    other._initialized = false;
    other._iv_length = 0;
    pt_recursive_mutex_unlock_if_valid(other._mutex);
    ft_global_error_stack_drop_last_error();
    pt_recursive_mutex_unlock_if_valid(this->_mutex);
    ft_global_error_stack_drop_last_error();
    return ;
}

encryption_aead_context &encryption_aead_context::operator=(encryption_aead_context &&other) noexcept
{
    if (this == &other)
        return (*this);
    int self_lock = pt_recursive_mutex_lock_if_valid(this->_mutex);
    ft_global_error_stack_drop_last_error();
    if (self_lock != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(self_lock);
        return (*this);
    }
    int other_lock = pt_recursive_mutex_lock_if_valid(other._mutex);
    ft_global_error_stack_drop_last_error();
    if (other_lock != FT_ERR_SUCCESSS)
    {
        pt_recursive_mutex_unlock_if_valid(this->_mutex);
        ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(other_lock);
        return (*this);
    }
    if (this->_context != NULL)
        EVP_CIPHER_CTX_free(this->_context);
    this->_context = other._context;
    this->_cipher = other._cipher;
    this->_encrypt_mode = other._encrypt_mode;
    this->_initialized = other._initialized;
    this->_iv_length = other._iv_length;
    other._context = NULL;
    other._cipher = NULL;
    other._encrypt_mode = false;
    other._initialized = false;
    other._iv_length = 0;
    pt_recursive_mutex_unlock_if_valid(other._mutex);
    ft_global_error_stack_drop_last_error();
    pt_recursive_mutex_unlock_if_valid(this->_mutex);
    ft_global_error_stack_drop_last_error();
    return (*this);
}

int encryption_aead_context::configure_cipher(const unsigned char *key, size_t key_length,
        const unsigned char *iv, size_t iv_length, bool encrypt_mode)
{
    const EVP_CIPHER *cipher;
    int control_result;
    int iv_length_value;
    int encrypt_flag = 0;
    int lock_result = pt_recursive_mutex_lock_if_valid(this->_mutex);
    ft_global_error_stack_drop_last_error();

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (lock_result);
    }
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
    return (this->finalize_operation(FT_ERR_SUCCESSS));
}

int encryption_aead_context::initialize_encrypt(const unsigned char *key, size_t key_length,
        const unsigned char *iv, size_t iv_length)
{
    return (this->configure_cipher(key, key_length, iv, iv_length, true));
}

int encryption_aead_context::initialize_decrypt(const unsigned char *key, size_t key_length,
        const unsigned char *iv, size_t iv_length)
{
    return (this->configure_cipher(key, key_length, iv, iv_length, false));
}

int encryption_aead_context::update_aad(const unsigned char *aad, size_t aad_length)
{
    int lock_result = pt_recursive_mutex_lock_if_valid(this->_mutex);
    ft_global_error_stack_drop_last_error();

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (lock_result);
    }
    if (!this->_initialized)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    if (aad_length == 0)
        return (this->finalize_operation(FT_ERR_SUCCESSS));
    int update_result;
    int out_length = 0;

    update_result = EVP_CipherUpdate(this->_context, NULL, &out_length,
            aad, static_cast<int>(aad_length));
    if (update_result != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    return (this->finalize_operation(FT_ERR_SUCCESSS));
}

int encryption_aead_context::update(const unsigned char *input, size_t input_length,
        unsigned char *output, size_t &output_length)
{
    int lock_result = pt_recursive_mutex_lock_if_valid(this->_mutex);
    ft_global_error_stack_drop_last_error();

    output_length = 0;
    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (lock_result);
    }
    if (!this->_initialized)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    if (input_length == 0)
        return (this->finalize_operation(FT_ERR_SUCCESSS));
    int local_length = 0;
    int update_result = EVP_CipherUpdate(this->_context, output, &local_length,
            input, static_cast<int>(input_length));
    if (update_result != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    if (local_length < 0)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    output_length = static_cast<size_t>(local_length);
    return (this->finalize_operation(FT_ERR_SUCCESSS));
}

int encryption_aead_context::finalize(unsigned char *tag, size_t tag_length)
{
    int lock_result = pt_recursive_mutex_lock_if_valid(this->_mutex);
    ft_global_error_stack_drop_last_error();

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (lock_result);
    }
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
        return (this->finalize_operation(FT_ERR_SUCCESSS));
    }
    if (tag == NULL)
        return (this->finalize_operation(FT_ERR_INVALID_ARGUMENT));
    if (tag_length == 0 || tag_length > 16)
        return (this->finalize_operation(FT_ERR_INVALID_ARGUMENT));
    if (EVP_CIPHER_CTX_ctrl(this->_context, EVP_CTRL_GCM_GET_TAG,
            static_cast<int>(tag_length), tag) != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    this->_initialized = false;
    return (this->finalize_operation(FT_ERR_SUCCESSS));
}

int encryption_aead_context::set_tag(const unsigned char *tag, size_t tag_length)
{
    int lock_result = pt_recursive_mutex_lock_if_valid(this->_mutex);
    ft_global_error_stack_drop_last_error();

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (lock_result);
    }
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
    return (this->finalize_operation(FT_ERR_SUCCESSS));
}

void    encryption_aead_context::reset()
{
    int lock_result = pt_recursive_mutex_lock_if_valid(this->_mutex);
    ft_global_error_stack_drop_last_error();

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    if (this->_context != NULL)
        EVP_CIPHER_CTX_reset(this->_context);
    this->_cipher = NULL;
    this->_encrypt_mode = false;
    this->_initialized = false;
    this->_iv_length = 0;
    int unlock_result = pt_recursive_mutex_unlock_if_valid(this->_mutex);
    ft_global_error_stack_drop_last_error();
    if (unlock_result != FT_ERR_SUCCESSS)
        return ;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

bool    encryption_aead_encrypt(const unsigned char *key, size_t key_length,
        const unsigned char *iv, size_t iv_length,
        const unsigned char *aad, size_t aad_length,
        const unsigned char *plaintext, size_t plaintext_length,
        unsigned char *ciphertext, unsigned char *tag, size_t tag_length)
{
    encryption_aead_context context;
    size_t output_length;

    if (context.initialize_encrypt(key, key_length, iv, iv_length) != FT_ERR_SUCCESSS)
        return (false);
    if (context.update_aad(aad, aad_length) != FT_ERR_SUCCESSS)
        return (false);
    output_length = 0;
    if (plaintext_length > 0)
    {
        if (ciphertext == NULL)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            return (false);
        }
        if (context.update(plaintext, plaintext_length, ciphertext,
                output_length) != FT_ERR_SUCCESSS)
            return (false);
        if (output_length != plaintext_length)
        {
            ft_global_error_stack_push(FT_ERR_INTERNAL);
            return (false);
        }
    }
    if (context.finalize(tag, tag_length) != FT_ERR_SUCCESSS)
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

    if (context.initialize_decrypt(key, key_length, iv, iv_length) != FT_ERR_SUCCESSS)
        return (false);
    if (context.set_tag(tag, tag_length) != FT_ERR_SUCCESSS)
        return (false);
    if (context.update_aad(aad, aad_length) != FT_ERR_SUCCESSS)
        return (false);
    output_length = 0;
    if (ciphertext_length > 0)
    {
        if (plaintext == NULL)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            return (false);
        }
        if (context.update(ciphertext, ciphertext_length, plaintext,
                output_length) != FT_ERR_SUCCESSS)
            return (false);
        if (output_length != ciphertext_length)
        {
            ft_global_error_stack_push(FT_ERR_INTERNAL);
            return (false);
        }
    }
    if (context.finalize(NULL, 0) != FT_ERR_SUCCESSS)
        return (false);
    return (true);
}

#endif // NETWORKING_HAS_OPENSSL
