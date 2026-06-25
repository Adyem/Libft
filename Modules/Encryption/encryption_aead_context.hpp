#ifndef ENCRYPTION_AEAD_CONTEXT_HPP
#define ENCRYPTION_AEAD_CONTEXT_HPP

#include <cstddef>
#include <cstdint>
#include "../Networking/openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL
#include <openssl/evp.h>
#include "../PThread/recursive_mutex.hpp"
#include "../Errno/errno.hpp"

class encryption_aead_context
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        EVP_CIPHER_CTX *_context;
        const EVP_CIPHER *_cipher;
        ft_bool _encrypt_mode;
        ft_bool _initialised;
        ft_size_t _initialization_vector_length;
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialised_state;

        int32_t finalize_operation(int32_t result_value) const;
        int32_t configure_cipher(const uint8_t *key_buffer,
                    ft_size_t key_length, const uint8_t *initialization_vector,
                    ft_size_t initialization_vector_length,
                    ft_bool encrypt_mode);

    public:
        encryption_aead_context() noexcept;
        encryption_aead_context(const encryption_aead_context &other) noexcept = delete;
        encryption_aead_context(encryption_aead_context &&other) noexcept = delete;
        ~encryption_aead_context() noexcept;

        encryption_aead_context &operator=(const encryption_aead_context &other) = delete;
        encryption_aead_context &operator=(encryption_aead_context &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t initialize(const encryption_aead_context &other) noexcept;
        int32_t initialize(encryption_aead_context &&other) noexcept;
        int32_t destroy() noexcept;
        uint32_t move(encryption_aead_context &other) noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;

        int32_t initialize_encrypt(const uint8_t *key_buffer,
                    ft_size_t key_length,
                    const uint8_t *initialization_vector,
                    ft_size_t initialization_vector_length);
        int32_t initialize_decrypt(const uint8_t *key_buffer,
                    ft_size_t key_length,
                    const uint8_t *initialization_vector,
                    ft_size_t initialization_vector_length);
        int32_t update_aad(
                    const uint8_t *additional_authenticated_data_buffer,
                    ft_size_t additional_authenticated_data_length);
        int32_t update(const uint8_t *input_buffer, ft_size_t input_length,
                    uint8_t *output_buffer, ft_size_t &output_length);
        int32_t finalize(uint8_t *authentication_tag_buffer,
                    ft_size_t authentication_tag_length);
        int32_t set_tag(const uint8_t *authentication_tag_buffer,
                    ft_size_t authentication_tag_length);
        void    reset() noexcept;
};

ft_bool encryption_aead_encrypt(const uint8_t *key_buffer,
            ft_size_t key_length,
            const uint8_t *initialization_vector,
            ft_size_t initialization_vector_length,
            const uint8_t *additional_authenticated_data_buffer,
            ft_size_t additional_authenticated_data_length,
            const uint8_t *plaintext_buffer, ft_size_t plaintext_length,
            uint8_t *ciphertext_buffer,
            uint8_t *authentication_tag_buffer,
            ft_size_t authentication_tag_length);

ft_bool encryption_aead_decrypt(const uint8_t *key_buffer,
            ft_size_t key_length,
            const uint8_t *initialization_vector,
            ft_size_t initialization_vector_length,
            const uint8_t *additional_authenticated_data_buffer,
            ft_size_t additional_authenticated_data_length,
            const uint8_t *ciphertext_buffer, ft_size_t ciphertext_length,
            const uint8_t *authentication_tag_buffer,
            ft_size_t authentication_tag_length,
            uint8_t *plaintext_buffer);

#endif

#endif
