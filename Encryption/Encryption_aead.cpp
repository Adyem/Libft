#include "encryption_aead_context.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/pthread_internal.hpp"
#include <new>

#if NETWORKING_HAS_OPENSSL

static const EVP_CIPHER *encryption_aead_select_cipher(ft_size_t key_length)
{
    if (key_length == 16)
        return (EVP_aes_128_gcm());
    if (key_length == 24)
        return (EVP_aes_192_gcm());
    if (key_length == 32)
        return (EVP_aes_256_gcm());
    return (ft_nullptr);
}

encryption_aead_context::encryption_aead_context() noexcept
{
    this->_context = ft_nullptr;
    this->_cipher = ft_nullptr;
    this->_encrypt_mode = FT_FALSE;
    this->_initialised = FT_FALSE;
    this->_initialization_vector_length = 0;
    this->_mutex = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    return ;
}

encryption_aead_context::encryption_aead_context(
    const encryption_aead_context &other) noexcept
{
    int32_t initialize_result;
    int32_t lock_result;
    int32_t enable_result;

    this->_context = ft_nullptr;
    this->_cipher = ft_nullptr;
    this->_encrypt_mode = FT_FALSE;
    this->_initialised = FT_FALSE;
    this->_initialization_vector_length = 0;
    this->_mutex = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "encryption_aead_context::encryption_aead_context(const encryption_aead_context&)",
            "source object is uninitialised");
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    initialize_result = this->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    lock_result = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_result != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._context == ft_nullptr)
    {
        (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (EVP_CIPHER_CTX_copy(this->_context, other._context) != 1)
    {
        (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    this->_cipher = other._cipher;
    this->_encrypt_mode = other._encrypt_mode;
    this->_initialised = other._initialised;
    this->_initialization_vector_length = other._initialization_vector_length;
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    if (other._mutex != ft_nullptr)
    {
        enable_result = this->enable_thread_safety();
        if (enable_result != FT_ERR_SUCCESS)
        {
            (void)this->destroy();
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return ;
        }
    }
    return ;
}

encryption_aead_context::encryption_aead_context(
    encryption_aead_context &&other) noexcept
{
    pt_recursive_mutex *source_mutex;
    int32_t lock_result;

    this->_context = ft_nullptr;
    this->_cipher = ft_nullptr;
    this->_encrypt_mode = FT_FALSE;
    this->_initialised = FT_FALSE;
    this->_initialization_vector_length = 0;
    this->_mutex = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "encryption_aead_context::encryption_aead_context(encryption_aead_context&&)",
            "source object is uninitialised");
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    source_mutex = other._mutex;
    lock_result = pt_recursive_mutex_lock_if_not_null(source_mutex);
    if (lock_result != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    this->_context = other._context;
    this->_cipher = other._cipher;
    this->_encrypt_mode = other._encrypt_mode;
    this->_initialised = other._initialised;
    this->_initialization_vector_length = other._initialization_vector_length;
    this->_mutex = other._mutex;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._context = ft_nullptr;
    other._cipher = ft_nullptr;
    other._encrypt_mode = FT_FALSE;
    other._initialised = FT_FALSE;
    other._initialization_vector_length = 0;
    other._mutex = ft_nullptr;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    (void)pt_recursive_mutex_unlock_if_not_null(source_mutex);
    return ;
}

encryption_aead_context::~encryption_aead_context() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

uint32_t encryption_aead_context::move(encryption_aead_context &other) noexcept
{
    int32_t destroy_result;
    pt_recursive_mutex *source_mutex;
    int32_t lock_result;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "encryption_aead_context::move",
            "source object is uninitialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_result = this->destroy();
        if (destroy_result != FT_ERR_SUCCESS)
            return (destroy_result);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_context = ft_nullptr;
        this->_cipher = ft_nullptr;
        this->_encrypt_mode = FT_FALSE;
        this->_initialised = FT_FALSE;
        this->_initialization_vector_length = 0;
        this->_mutex = ft_nullptr;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    source_mutex = other._mutex;
    lock_result = pt_recursive_mutex_lock_if_not_null(source_mutex);
    if (lock_result != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (static_cast<uint32_t>(lock_result));
    }
    this->_context = other._context;
    this->_cipher = other._cipher;
    this->_encrypt_mode = other._encrypt_mode;
    this->_initialised = other._initialised;
    this->_initialization_vector_length = other._initialization_vector_length;
    this->_mutex = other._mutex;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._context = ft_nullptr;
    other._cipher = ft_nullptr;
    other._encrypt_mode = FT_FALSE;
    other._initialised = FT_FALSE;
    other._initialization_vector_length = 0;
    other._mutex = ft_nullptr;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    (void)pt_recursive_mutex_unlock_if_not_null(source_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t encryption_aead_context::enable_thread_safety() noexcept
{
    pt_recursive_mutex *new_mutex;
    int32_t mutex_error;

    errno_abort_if_uninitialised(this->_initialised_state,
        "encryption_aead_context::enable_thread_safety");
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

int32_t encryption_aead_context::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    errno_abort_if_uninitialised(this->_initialised_state,
        "encryption_aead_context::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool encryption_aead_context::is_thread_safe() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "encryption_aead_context::is_thread_safe");
    if (this->_mutex != ft_nullptr)
        return (FT_TRUE);
    return (FT_FALSE);
}

int32_t encryption_aead_context::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state,
            "encryption_aead_context::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->_context = EVP_CIPHER_CTX_new();
    if (this->_context == ft_nullptr)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_NO_MEMORY);
    }
    return (FT_ERR_SUCCESS);
}

int32_t encryption_aead_context::initialize(
    const encryption_aead_context &other) noexcept
{
    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "encryption_aead_context::initialize(const encryption_aead_context&)",
            "source object is uninitialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        if (this->destroy() != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_context = ft_nullptr;
        this->_cipher = ft_nullptr;
        this->_encrypt_mode = FT_FALSE;
        this->_initialised = FT_FALSE;
        this->_initialization_vector_length = 0;
        this->_mutex = ft_nullptr;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    {
        encryption_aead_context temporary_context(other);
        if (temporary_context._initialised_state != FT_CLASS_STATE_INITIALISED)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (FT_ERR_INTERNAL);
        }
        return (static_cast<int32_t>(this->move(temporary_context)));
    }
}

int32_t encryption_aead_context::initialize(
    encryption_aead_context &&other) noexcept
{
    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "encryption_aead_context::initialize(encryption_aead_context&&)",
            "source object is uninitialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        if (this->destroy() != FT_ERR_SUCCESS)
            return (FT_ERR_INTERNAL);
    }
    return (static_cast<int32_t>(this->move(other)));
}

int32_t encryption_aead_context::destroy() noexcept
{
    int32_t first_error;
    int32_t disable_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    first_error = FT_ERR_SUCCESS;
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS && first_error == FT_ERR_SUCCESS)
        first_error = disable_error;
    if (this->_context != ft_nullptr)
    {
        EVP_CIPHER_CTX_free(this->_context);
        this->_context = ft_nullptr;
    }
    this->_cipher = ft_nullptr;
    this->_encrypt_mode = FT_FALSE;
    this->_initialised = FT_FALSE;
    this->_initialization_vector_length = 0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (first_error);
}

int32_t encryption_aead_context::finalize_operation(
    int32_t result_value) const
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "encryption_aead_context::finalize_operation");
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (result_value);
}

int32_t encryption_aead_context::configure_cipher(const uint8_t *key_buffer,
    ft_size_t key_length, const uint8_t *initialization_vector, ft_size_t initialization_vector_length,
    ft_bool encrypt_mode)
{
    const EVP_CIPHER *cipher;
    int32_t control_result;
    int32_t initialization_vector_length_value;
    int32_t encrypt_flag;
    int32_t lock_result;

    errno_abort_if_uninitialised(this->_initialised_state,
        "encryption_aead_context::configure_cipher");
    lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return (lock_result);
    if (this->_context == ft_nullptr)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    cipher = encryption_aead_select_cipher(key_length);
    if (cipher == ft_nullptr)
        return (this->finalize_operation(FT_ERR_UNSUPPORTED_TYPE));
    if (EVP_CIPHER_CTX_reset(this->_context) != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    encrypt_flag = 0;
    if (encrypt_mode == FT_TRUE)
        encrypt_flag = 1;
    if (EVP_CipherInit_ex(this->_context, cipher, ft_nullptr, ft_nullptr,
            ft_nullptr, encrypt_flag) != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    initialization_vector_length_value = EVP_CIPHER_iv_length(cipher);
    if (initialization_vector_length_value < 0)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    this->_initialization_vector_length = static_cast<ft_size_t>(initialization_vector_length_value);
    if (initialization_vector != ft_nullptr && initialization_vector_length != this->_initialization_vector_length)
    {
        control_result = EVP_CIPHER_CTX_ctrl(this->_context,
                EVP_CTRL_GCM_SET_IVLEN, static_cast<int32_t>(initialization_vector_length),
                ft_nullptr);
        if (control_result != 1)
            return (this->finalize_operation(FT_ERR_INVALID_ARGUMENT));
        this->_initialization_vector_length = initialization_vector_length;
    }
    if (EVP_CipherInit_ex(this->_context, ft_nullptr, ft_nullptr, key_buffer,
            initialization_vector,
            -1) != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    this->_cipher = cipher;
    this->_encrypt_mode = encrypt_mode;
    this->_initialised = FT_TRUE;
    return (this->finalize_operation(FT_ERR_SUCCESS));
}

int32_t encryption_aead_context::initialize_encrypt(const uint8_t *key_buffer,
    ft_size_t key_length, const uint8_t *initialization_vector, ft_size_t initialization_vector_length)
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "encryption_aead_context::initialize_encrypt");
    return (this->configure_cipher(key_buffer, key_length, initialization_vector, initialization_vector_length, FT_TRUE));
}

int32_t encryption_aead_context::initialize_decrypt(const uint8_t *key_buffer,
    ft_size_t key_length, const uint8_t *initialization_vector, ft_size_t initialization_vector_length)
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "encryption_aead_context::initialize_decrypt");
    return (this->configure_cipher(key_buffer, key_length, initialization_vector, initialization_vector_length, FT_FALSE));
}

int32_t encryption_aead_context::update_aad(const uint8_t *additional_authenticated_data_buffer,
    ft_size_t additional_authenticated_data_length)
{
    int32_t lock_result;
    int32_t update_result;
    int32_t out_length;

    errno_abort_if_uninitialised(this->_initialised_state,
        "encryption_aead_context::update_aad");
    lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return (lock_result);
    if (this->_initialised == FT_FALSE)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    if (additional_authenticated_data_length == 0)
        return (this->finalize_operation(FT_ERR_SUCCESS));
    out_length = 0;
    update_result = EVP_CipherUpdate(this->_context, ft_nullptr, &out_length,
            additional_authenticated_data_buffer, static_cast<int32_t>(additional_authenticated_data_length));
    if (update_result != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    return (this->finalize_operation(FT_ERR_SUCCESS));
}

int32_t encryption_aead_context::update(const uint8_t *input_buffer,
    ft_size_t input_length, uint8_t *output_buffer, ft_size_t &output_length)
{
    int32_t lock_result;
    int32_t local_length;
    int32_t update_result;

    errno_abort_if_uninitialised(this->_initialised_state,
        "encryption_aead_context::update");
    lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    output_length = 0;
    if (lock_result != FT_ERR_SUCCESS)
        return (lock_result);
    if (this->_initialised == FT_FALSE)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    if (input_length == 0)
        return (this->finalize_operation(FT_ERR_SUCCESS));
    local_length = 0;
    update_result = EVP_CipherUpdate(this->_context, output_buffer,
            &local_length, input_buffer, static_cast<int32_t>(input_length));
    if (update_result != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    if (local_length < 0)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    output_length = static_cast<ft_size_t>(local_length);
    return (this->finalize_operation(FT_ERR_SUCCESS));
}

int32_t encryption_aead_context::finalize(uint8_t *authentication_tag_buffer, ft_size_t authentication_tag_length)
{
    int32_t lock_result;
    int32_t final_result;
    uint8_t final_block[EVP_MAX_BLOCK_LENGTH];
    int32_t final_length;

    errno_abort_if_uninitialised(this->_initialised_state,
        "encryption_aead_context::finalize");
    lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return (lock_result);
    if (this->_initialised == FT_FALSE)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    final_result = 0;
    final_length = 0;
    ft_memset(final_block, 0, sizeof(final_block));
    final_result = EVP_CipherFinal_ex(this->_context, final_block,
            &final_length);
    if (final_result != 1)
    {
        if (this->_encrypt_mode == FT_TRUE)
            return (this->finalize_operation(FT_ERR_INTERNAL));
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    }
    if (final_length < 0)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    if (final_length != 0)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    if (this->_encrypt_mode == FT_FALSE)
    {
        this->_initialised = FT_FALSE;
        return (this->finalize_operation(FT_ERR_SUCCESS));
    }
    if (authentication_tag_buffer == ft_nullptr)
        return (this->finalize_operation(FT_ERR_INVALID_ARGUMENT));
    if (authentication_tag_length == 0 || authentication_tag_length > 16)
        return (this->finalize_operation(FT_ERR_INVALID_ARGUMENT));
    if (EVP_CIPHER_CTX_ctrl(this->_context, EVP_CTRL_GCM_GET_TAG,
            static_cast<int32_t>(authentication_tag_length), authentication_tag_buffer) != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    this->_initialised = FT_FALSE;
    return (this->finalize_operation(FT_ERR_SUCCESS));
}

int32_t encryption_aead_context::set_tag(const uint8_t *authentication_tag_buffer,
    ft_size_t authentication_tag_length)
{
    int32_t lock_result;

    errno_abort_if_uninitialised(this->_initialised_state,
        "encryption_aead_context::set_tag");
    lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return (lock_result);
    if (this->_initialised == FT_FALSE)
        return (this->finalize_operation(FT_ERR_INVALID_STATE));
    if (this->_encrypt_mode == FT_TRUE)
        return (this->finalize_operation(FT_ERR_INVALID_OPERATION));
    if (authentication_tag_buffer == ft_nullptr)
        return (this->finalize_operation(FT_ERR_INVALID_ARGUMENT));
    if (authentication_tag_length == 0 || authentication_tag_length > 16)
        return (this->finalize_operation(FT_ERR_INVALID_ARGUMENT));
    if (EVP_CIPHER_CTX_ctrl(this->_context, EVP_CTRL_GCM_SET_TAG,
            static_cast<int32_t>(authentication_tag_length), const_cast<uint8_t *>(authentication_tag_buffer)) != 1)
        return (this->finalize_operation(FT_ERR_INTERNAL));
    return (this->finalize_operation(FT_ERR_SUCCESS));
}

void encryption_aead_context::reset() noexcept
{
    int32_t lock_result;

    errno_abort_if_uninitialised(this->_initialised_state,
        "encryption_aead_context::reset");
    lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return ;
    if (this->_context != ft_nullptr)
        EVP_CIPHER_CTX_reset(this->_context);
    this->_cipher = ft_nullptr;
    this->_encrypt_mode = FT_FALSE;
    this->_initialised = FT_FALSE;
    this->_initialization_vector_length = 0;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

ft_bool encryption_aead_encrypt(const uint8_t *key_buffer, ft_size_t key_length,
    const uint8_t *initialization_vector, ft_size_t initialization_vector_length, const uint8_t *additional_authenticated_data_buffer,
    ft_size_t additional_authenticated_data_length, const uint8_t *plaintext_buffer, ft_size_t plaintext_length,
    uint8_t *ciphertext_buffer, uint8_t *authentication_tag_buffer, ft_size_t authentication_tag_length)
{
    encryption_aead_context context;
    ft_size_t output_length;

    if (context.initialize() != FT_ERR_SUCCESS)
        return (FT_FALSE);
    if (context.initialize_encrypt(key_buffer, key_length, initialization_vector, initialization_vector_length)
        != FT_ERR_SUCCESS)
        return (FT_FALSE);
    if (context.update_aad(additional_authenticated_data_buffer, additional_authenticated_data_length) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    output_length = 0;
    if (plaintext_length > 0)
    {
        if (ciphertext_buffer == ft_nullptr)
            return (FT_FALSE);
        if (context.update(plaintext_buffer, plaintext_length,
                ciphertext_buffer,
                output_length) != FT_ERR_SUCCESS)
            return (FT_FALSE);
        if (output_length != plaintext_length)
            return (FT_FALSE);
    }
    if (context.finalize(authentication_tag_buffer, authentication_tag_length) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    return (FT_TRUE);
}

ft_bool encryption_aead_decrypt(const uint8_t *key_buffer, ft_size_t key_length,
    const uint8_t *initialization_vector, ft_size_t initialization_vector_length, const uint8_t *additional_authenticated_data_buffer,
    ft_size_t additional_authenticated_data_length, const uint8_t *ciphertext_buffer,
    ft_size_t ciphertext_length, const uint8_t *authentication_tag_buffer, ft_size_t authentication_tag_length,
    uint8_t *plaintext_buffer)
{
    encryption_aead_context context;
    ft_size_t output_length;

    if (context.initialize() != FT_ERR_SUCCESS)
        return (FT_FALSE);
    if (context.initialize_decrypt(key_buffer, key_length, initialization_vector, initialization_vector_length)
        != FT_ERR_SUCCESS)
        return (FT_FALSE);
    if (context.set_tag(authentication_tag_buffer, authentication_tag_length) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    if (context.update_aad(additional_authenticated_data_buffer, additional_authenticated_data_length) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    output_length = 0;
    if (ciphertext_length > 0)
    {
        if (plaintext_buffer == ft_nullptr)
            return (FT_FALSE);
        if (context.update(ciphertext_buffer, ciphertext_length,
                plaintext_buffer,
                output_length) != FT_ERR_SUCCESS)
            return (FT_FALSE);
        if (output_length != ciphertext_length)
            return (FT_FALSE);
    }
    if (context.finalize(ft_nullptr, 0) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    return (FT_TRUE);
}

#endif // NETWORKING_HAS_OPENSSL
