#ifndef NETWORKING_QUIC_EXPERIMENTAL_HPP
#define NETWORKING_QUIC_EXPERIMENTAL_HPP

#include "ssl_wrapper.hpp"
#include "networking_tls_aead.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/vector.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../Encryption/encryption_aead.hpp"
#include "openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL

struct quic_feature_configuration
{
    bool    enable_datagram_pacing;
    bool    enable_loss_recovery;

    quic_feature_configuration() noexcept;
    ~quic_feature_configuration() noexcept;
};

struct quic_datagram_plaintext
{
    const unsigned char   *payload;
    size_t                payload_length;
    const unsigned char   *associated_data;
    size_t                associated_data_length;

    quic_datagram_plaintext() noexcept;
    ~quic_datagram_plaintext() noexcept;
};

bool    networking_quic_enable_experimental() noexcept;
bool    networking_quic_disable_experimental() noexcept;
bool    networking_quic_is_experimental_enabled() noexcept;

class quic_experimental_session
{
    private:
        void        set_error(int error_code) const noexcept;
        void        clear_key_material() noexcept;
        bool        ensure_feature_enabled() const noexcept;
        bool        ensure_configured() const noexcept;
        bool        derive_keys(SSL *ssl_session, bool outbound) noexcept;
        bool        prepare_nonce(uint64_t sequence_number,
                        const ft_vector<unsigned char> &base_iv,
                        ft_vector<unsigned char> &out_nonce) noexcept;

        SSL                             *_ssl_session;
        bool                            _outbound;
        bool                            _configured;
        quic_feature_configuration      _feature_configuration;
        ft_vector<unsigned char>        _send_key;
        ft_vector<unsigned char>        _receive_key;
        ft_vector<unsigned char>        _send_iv;
        ft_vector<unsigned char>        _receive_iv;
        uint64_t                        _send_sequence;
        uint64_t                        _receive_sequence;
        mutable int                     _error_code;

    public:
        quic_experimental_session() noexcept;
        ~quic_experimental_session() noexcept;

        bool    configure(SSL *ssl_session,
                    const quic_feature_configuration &configuration,
                    bool outbound) noexcept;
        bool    encrypt_datagram(const quic_datagram_plaintext &plaintext,
                    ft_vector<unsigned char> &out_ciphertext) noexcept;
        bool    decrypt_datagram(const ft_vector<unsigned char> &ciphertext,
                    const unsigned char *associated_data,
                    size_t associated_data_length,
                    ft_vector<unsigned char> &out_plaintext) noexcept;
        bool    get_feature_configuration(quic_feature_configuration &out_configuration) const noexcept;
        int     get_error() const noexcept;
        const char  *get_error_str() const noexcept;
};

#endif
#endif
