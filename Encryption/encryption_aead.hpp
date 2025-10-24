#ifndef ENCRYPTION_AEAD_HPP
#define ENCRYPTION_AEAD_HPP

#include <cstddef>
#include <openssl/evp.h>

class encryption_aead_context
{
    private:
        EVP_CIPHER_CTX *_context;
        const EVP_CIPHER *_cipher;
        bool _encrypt_mode;
        bool _initialized;
        size_t _iv_length;
        mutable int _error_code;

        void    set_error(int error_code) const;
        int     configure_cipher(const unsigned char *key, size_t key_length,
                    const unsigned char *iv, size_t iv_length, bool encrypt_mode);

    public:
        encryption_aead_context();
        ~encryption_aead_context();

        encryption_aead_context(const encryption_aead_context &other) = delete;
        encryption_aead_context &operator=(const encryption_aead_context &other) = delete;

        encryption_aead_context(encryption_aead_context &&other) noexcept;
        encryption_aead_context &operator=(encryption_aead_context &&other) noexcept;

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
        int     get_error() const;
        const char  *get_error_str() const;
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
