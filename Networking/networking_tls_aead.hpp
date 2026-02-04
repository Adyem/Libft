#ifndef NETWORKING_TLS_AEAD_HPP
#define NETWORKING_TLS_AEAD_HPP

#include "ssl_wrapper.hpp"
#include "../Template/vector.hpp"
#include "../Encryption/encryption_aead.hpp"
#include "openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL
bool    networking_tls_export_aead_keys(SSL *ssl_session, bool outbound,
            ft_vector<unsigned char> &send_key,
            ft_vector<unsigned char> &send_iv,
            ft_vector<unsigned char> &receive_key,
            ft_vector<unsigned char> &receive_iv);

bool    networking_tls_initialize_aead_contexts(SSL *ssl_session, bool outbound,
            encryption_aead_context &send_context,
            encryption_aead_context &receive_context,
            ft_vector<unsigned char> &send_iv,
            ft_vector<unsigned char> &receive_iv);
#endif

#endif
