#include "encryption_aead.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

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
    this->_context = EVP_CIPHER_CTX_new();
    this->_cipher = NULL;
    this->_encrypt_mode = false;
    this->_initialized = false;
    this->_iv_length = 0;
    this->_error_code = ER_SUCCESS;
    if (this->_context == NULL)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

encryption_aead_context::~encryption_aead_context()
{
    if (this->_context != NULL)
    {
        EVP_CIPHER_CTX_free(this->_context);
        this->_context = NULL;
    }
    return ;
}

void    encryption_aead_context::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

encryption_aead_context::encryption_aead_context(encryption_aead_context &&other) noexcept
{
    this->_context = other._context;
    this->_cipher = other._cipher;
    this->_encrypt_mode = other._encrypt_mode;
    this->_initialized = other._initialized;
    this->_iv_length = other._iv_length;
    this->_error_code = other._error_code;
    other._context = NULL;
    other._cipher = NULL;
    other._encrypt_mode = false;
    other._initialized = false;
    other._iv_length = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

encryption_aead_context &encryption_aead_context::operator=(encryption_aead_context &&other) noexcept
{
    if (this == &other)
        return (*this);
    if (this->_context != NULL)
        EVP_CIPHER_CTX_free(this->_context);
    this->_context = other._context;
    this->_cipher = other._cipher;
    this->_encrypt_mode = other._encrypt_mode;
    this->_initialized = other._initialized;
    this->_iv_length = other._iv_length;
    this->_error_code = other._error_code;
    other._context = NULL;
    other._cipher = NULL;
    other._encrypt_mode = false;
    other._initialized = false;
    other._iv_length = 0;
    other._error_code = ER_SUCCESS;
    return (*this);
}

int encryption_aead_context::configure_cipher(const unsigned char *key, size_t key_length,
        const unsigned char *iv, size_t iv_length, bool encrypt_mode)
{
    const EVP_CIPHER *cipher;
    int control_result;
    int iv_length_value;

    if (this->_context == NULL)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    cipher = encryption_aead_select_cipher(key_length);
    if (cipher == NULL)
    {
        this->set_error(FT_ERR_UNSUPPORTED_TYPE);
        return (FT_ERR_UNSUPPORTED_TYPE);
    }
    if (EVP_CIPHER_CTX_reset(this->_context) != 1)
    {
        this->set_error(FT_ERR_INTERNAL);
        return (FT_ERR_INTERNAL);
    }
    if (EVP_CipherInit_ex(this->_context, cipher, NULL, NULL, NULL,
            encrypt_mode ? 1 : 0) != 1)
    {
        this->set_error(FT_ERR_INTERNAL);
        return (FT_ERR_INTERNAL);
    }
    iv_length_value = EVP_CIPHER_iv_length(cipher);
    if (iv_length_value < 0)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_iv_length = static_cast<size_t>(iv_length_value);
    if (iv != NULL && iv_length != this->_iv_length)
    {
        control_result = EVP_CIPHER_CTX_ctrl(this->_context,
                EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(iv_length), NULL);
        if (control_result != 1)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (FT_ERR_INVALID_ARGUMENT);
        }
        this->_iv_length = iv_length;
    }
    if (EVP_CipherInit_ex(this->_context, NULL, NULL, key, iv, -1) != 1)
    {
        this->set_error(FT_ERR_INTERNAL);
        return (FT_ERR_INTERNAL);
    }
    this->_cipher = cipher;
    this->_encrypt_mode = encrypt_mode;
    this->_initialized = true;
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
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
    int update_result;
    int out_length;

    if (!this->_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (aad_length == 0)
    {
        this->set_error(ER_SUCCESS);
        return (ER_SUCCESS);
    }
    update_result = EVP_CipherUpdate(this->_context, NULL, &out_length,
            aad, static_cast<int>(aad_length));
    if (update_result != 1)
    {
        this->set_error(FT_ERR_INTERNAL);
        return (FT_ERR_INTERNAL);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int encryption_aead_context::update(const unsigned char *input, size_t input_length,
        unsigned char *output, size_t &output_length)
{
    int update_result;
    int local_length;

    output_length = 0;
    if (!this->_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (input_length == 0)
    {
        this->set_error(ER_SUCCESS);
        return (ER_SUCCESS);
    }
    update_result = EVP_CipherUpdate(this->_context, output, &local_length,
            input, static_cast<int>(input_length));
    if (update_result != 1)
    {
        this->set_error(FT_ERR_INTERNAL);
        return (FT_ERR_INTERNAL);
    }
    if (local_length < 0)
    {
        this->set_error(FT_ERR_INTERNAL);
        return (FT_ERR_INTERNAL);
    }
    output_length = static_cast<size_t>(local_length);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int encryption_aead_context::finalize(unsigned char *tag, size_t tag_length)
{
    int final_result;
    unsigned char final_block[EVP_MAX_BLOCK_LENGTH];
    int final_length;

    if (!this->_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    final_length = 0;
    ft_memset(final_block, 0, sizeof(final_block));
    final_result = EVP_CipherFinal_ex(this->_context, final_block, &final_length);
    if (final_result != 1)
    {
        if (this->_encrypt_mode)
        {
            this->set_error(FT_ERR_INTERNAL);
            return (FT_ERR_INTERNAL);
        }
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (final_length < 0)
    {
        this->set_error(FT_ERR_INTERNAL);
        return (FT_ERR_INTERNAL);
    }
    if (final_length != 0)
    {
        this->set_error(FT_ERR_INTERNAL);
        return (FT_ERR_INTERNAL);
    }
    if (!this->_encrypt_mode)
    {
        this->_initialized = false;
        this->set_error(ER_SUCCESS);
        return (ER_SUCCESS);
    }
    if (tag == NULL)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (tag_length == 0 || tag_length > 16)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (EVP_CIPHER_CTX_ctrl(this->_context, EVP_CTRL_GCM_GET_TAG,
            static_cast<int>(tag_length), tag) != 1)
    {
        this->set_error(FT_ERR_INTERNAL);
        return (FT_ERR_INTERNAL);
    }
    this->_initialized = false;
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int encryption_aead_context::set_tag(const unsigned char *tag, size_t tag_length)
{
    if (!this->_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_encrypt_mode)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (FT_ERR_INVALID_OPERATION);
    }
    if (tag == NULL)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (tag_length == 0 || tag_length > 16)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (EVP_CIPHER_CTX_ctrl(this->_context, EVP_CTRL_GCM_SET_TAG,
            static_cast<int>(tag_length), const_cast<unsigned char *>(tag)) != 1)
    {
        this->set_error(FT_ERR_INTERNAL);
        return (FT_ERR_INTERNAL);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

void    encryption_aead_context::reset()
{
    if (this->_context != NULL)
        EVP_CIPHER_CTX_reset(this->_context);
    this->_cipher = NULL;
    this->_encrypt_mode = false;
    this->_initialized = false;
    this->_iv_length = 0;
    this->set_error(ER_SUCCESS);
    return ;
}

int encryption_aead_context::get_error() const
{
    return (this->_error_code);
}

const char  *encryption_aead_context::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

bool    encryption_aead_encrypt(const unsigned char *key, size_t key_length,
        const unsigned char *iv, size_t iv_length,
        const unsigned char *aad, size_t aad_length,
        const unsigned char *plaintext, size_t plaintext_length,
        unsigned char *ciphertext, unsigned char *tag, size_t tag_length)
{
    encryption_aead_context context;
    size_t output_length;

    if (context.initialize_encrypt(key, key_length, iv, iv_length) != ER_SUCCESS)
        return (false);
    if (context.update_aad(aad, aad_length) != ER_SUCCESS)
        return (false);
    output_length = 0;
    if (plaintext_length > 0)
    {
        if (ciphertext == NULL)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (false);
        }
        if (context.update(plaintext, plaintext_length, ciphertext,
                output_length) != ER_SUCCESS)
            return (false);
        if (output_length != plaintext_length)
        {
            ft_errno = FT_ERR_INTERNAL;
            return (false);
        }
    }
    if (context.finalize(tag, tag_length) != ER_SUCCESS)
        return (false);
    ft_errno = ER_SUCCESS;
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

    if (context.initialize_decrypt(key, key_length, iv, iv_length) != ER_SUCCESS)
        return (false);
    if (context.set_tag(tag, tag_length) != ER_SUCCESS)
        return (false);
    if (context.update_aad(aad, aad_length) != ER_SUCCESS)
        return (false);
    output_length = 0;
    if (ciphertext_length > 0)
    {
        if (plaintext == NULL)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (false);
        }
        if (context.update(ciphertext, ciphertext_length, plaintext,
                output_length) != ER_SUCCESS)
            return (false);
        if (output_length != ciphertext_length)
        {
            ft_errno = FT_ERR_INTERNAL;
            return (false);
        }
    }
    if (context.finalize(NULL, 0) != ER_SUCCESS)
        return (false);
    ft_errno = ER_SUCCESS;
    return (true);
}
