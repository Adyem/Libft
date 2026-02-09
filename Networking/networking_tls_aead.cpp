#include "networking_tls_aead.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL

static const EVP_CIPHER *networking_tls_resolve_cipher(const SSL_CIPHER *cipher)
{
    if (cipher == NULL)
        return (NULL);
    int cipher_nid;

    cipher_nid = SSL_CIPHER_get_cipher_nid(cipher);
    if (cipher_nid == NID_aes_128_gcm)
        return (EVP_aes_128_gcm());
    if (cipher_nid == NID_aes_256_gcm)
        return (EVP_aes_256_gcm());
#ifdef NID_aes_192_gcm
    if (cipher_nid == NID_aes_192_gcm)
        return (EVP_aes_192_gcm());
#endif
    return (NULL);
}

static bool networking_tls_prepare_buffer(ft_vector<unsigned char> &buffer,
        size_t length)
{
    buffer.resize(length, 0);
    if (buffer.get_error() != FT_ERR_SUCCESSS)
        return (false);
    return (true);
}

bool    networking_tls_export_aead_keys(SSL *ssl_session, bool outbound,
        ft_vector<unsigned char> &send_key,
        ft_vector<unsigned char> &send_iv,
        ft_vector<unsigned char> &receive_key,
        ft_vector<unsigned char> &receive_iv)
{
    const SSL_CIPHER *cipher;
    const EVP_CIPHER *evp_cipher;
    int key_length_value;
    int iv_length_value;
    size_t key_length;
    size_t iv_length;
    size_t material_length;
    ft_vector<unsigned char> material;
    unsigned char *material_data;
    const char *label;
    size_t label_length;

    if (ssl_session == NULL)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    cipher = SSL_get_current_cipher(ssl_session);
    evp_cipher = networking_tls_resolve_cipher(cipher);
    if (evp_cipher == NULL)
    {
        ft_global_error_stack_push(FT_ERR_UNSUPPORTED_TYPE);
        return (false);
    }
    key_length_value = EVP_CIPHER_key_length(evp_cipher);
    iv_length_value = EVP_CIPHER_iv_length(evp_cipher);
    if (key_length_value <= 0 || iv_length_value <= 0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return (false);
    }
    key_length = static_cast<size_t>(key_length_value);
    iv_length = static_cast<size_t>(iv_length_value);
    material_length = (key_length + iv_length) * 2;
    if (!networking_tls_prepare_buffer(material, material_length))
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (false);
    }
    if (material.size() > 0)
        material_data = material.begin();
    else
        material_data = NULL;
    label = "EXPORTER-Libft-AEAD";
    label_length = ft_strlen(label);
    if (material_length > 0)
    {
        if (SSL_export_keying_material(ssl_session, material_data, material_length,
                label, label_length, NULL, 0, 0) != 1)
        {
            ft_global_error_stack_push(FT_ERR_INTERNAL);
            return (false);
        }
    }
    if (!networking_tls_prepare_buffer(send_key, key_length))
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (false);
    }
    if (!networking_tls_prepare_buffer(send_iv, iv_length))
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (false);
    }
    if (!networking_tls_prepare_buffer(receive_key, key_length))
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (false);
    }
    if (!networking_tls_prepare_buffer(receive_iv, iv_length))
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (false);
    }
    const unsigned char *cursor;
    const unsigned char *first_key;
    const unsigned char *first_iv;
    const unsigned char *second_key;
    const unsigned char *second_iv;

    cursor = material_data;
    first_key = cursor;
    first_iv = cursor + key_length;
    second_key = first_iv + iv_length;
    second_iv = second_key + key_length;
    if (outbound)
    {
        if (key_length > 0)
        {
            ft_memcpy(send_key.begin(), first_key, key_length);
            ft_memcpy(receive_key.begin(), second_key, key_length);
        }
        if (iv_length > 0)
        {
            ft_memcpy(send_iv.begin(), first_iv, iv_length);
            ft_memcpy(receive_iv.begin(), second_iv, iv_length);
        }
    }
    else
    {
        if (key_length > 0)
        {
            ft_memcpy(send_key.begin(), second_key, key_length);
            ft_memcpy(receive_key.begin(), first_key, key_length);
        }
        if (iv_length > 0)
        {
            ft_memcpy(send_iv.begin(), second_iv, iv_length);
            ft_memcpy(receive_iv.begin(), first_iv, iv_length);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

bool    networking_tls_initialize_aead_contexts(SSL *ssl_session, bool outbound,
        encryption_aead_context &send_context,
        encryption_aead_context &receive_context,
        ft_vector<unsigned char> &send_iv,
        ft_vector<unsigned char> &receive_iv)
{
    ft_vector<unsigned char> send_key;
    ft_vector<unsigned char> receive_key;

    if (!networking_tls_export_aead_keys(ssl_session, outbound, send_key,
            send_iv, receive_key, receive_iv))
        return (false);
    if (send_key.size() == 0 || receive_key.size() == 0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return (false);
    }
    if (send_iv.size() == 0 || receive_iv.size() == 0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return (false);
    }
    if (send_context.initialize_encrypt(send_key.begin(), send_key.size(),
            send_iv.begin(), send_iv.size()) != FT_ERR_SUCCESSS)
        return (false);
    if (receive_context.initialize_decrypt(receive_key.begin(),
            receive_key.size(), receive_iv.begin(), receive_iv.size()) != FT_ERR_SUCCESSS)
        return (false);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

#endif
