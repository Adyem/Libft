#ifndef ENCRYPTION_AEAD_HPP
#define ENCRYPTION_AEAD_HPP

#include <cstddef>
#include <cstdint>
#include "../Networking/openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL
#include <openssl/evp.h>
#include "../PThread/recursive_mutex.hpp"

class encryption_aead_context
{
    private:
        EVP_CIPHER_CTX *_context;
        const EVP_CIPHER *_cipher;
        bool _encrypt_mode;
        bool _initialized;
        size_t _iv_length;
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        int     finalize_operation(int result) const;
        int     configure_cipher(const unsigned char *key, size_t key_length,
                    const unsigned char *iv, size_t iv_length, bool encrypt_mode);
        void    abort_lifecycle_error(const char *method_name,
                    const char *reason) const;
        void    abort_if_not_initialized(const char *method_name) const;

    public:
        encryption_aead_context();
        ~encryption_aead_context();

        encryption_aead_context(const encryption_aead_context &other) = delete;
        encryption_aead_context &operator=(const encryption_aead_context &other) = delete;
        encryption_aead_context(encryption_aead_context &&other) noexcept = delete;
        encryption_aead_context &operator=(encryption_aead_context &&other) noexcept = delete;

        int     initialize();
        int     destroy();
        int     enable_thread_safety();
        int     disable_thread_safety();
        bool    is_thread_safe() const;

        int     initialize_encrypt(const unsigned char *key, size_t key_length,
                    const unsigned char *iv, size_t iv_length);
        int     initialize_decrypt(const unsigned char *key, size_t key_length,
                    const unsigned char *iv, size_t iv_length);
        int     update_aad(const unsigned char *aad, size_t aad_length);
        int     update(const unsigned char *input, size_t input_length,
                    unsigned char *output, size_t &output_length);
        int     finalize(unsigned char *tag, size_t tag_length);
        int     set_tag(const unsigned char *tag, size_t tag_length);
        void    reset();
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const;
#endif
};

bool    encryption_aead_encrypt(const unsigned char *key, size_t key_length,
            const unsigned char *iv, size_t iv_length,
            const unsigned char *aad, size_t aad_length,
            const unsigned char *plaintext, size_t plaintext_length,
            unsigned char *ciphertext, unsigned char *tag, size_t tag_length);

bool    encryption_aead_decrypt(const unsigned char *key, size_t key_length,
            const unsigned char *iv, size_t iv_length,
            const unsigned char *aad, size_t aad_length,
            const unsigned char *ciphertext, size_t ciphertext_length,
            const unsigned char *tag, size_t tag_length,
            unsigned char *plaintext);

#endif

#endif
