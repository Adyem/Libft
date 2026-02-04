#include "networking_quic_experimental.hpp"
#include "openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL

#include <atomic>

static std::atomic<bool> g_quic_experimental_enabled(false);

static const size_t QUIC_EXPERIMENTAL_TAG_LENGTH = 16;

quic_feature_configuration::quic_feature_configuration() noexcept
{
    this->enable_datagram_pacing = false;
    this->enable_loss_recovery = false;
    return ;
}

quic_feature_configuration::~quic_feature_configuration() noexcept
{
    return ;
}

quic_datagram_plaintext::quic_datagram_plaintext() noexcept
{
    this->payload = ft_nullptr;
    this->payload_length = 0;
    this->associated_data = ft_nullptr;
    this->associated_data_length = 0;
    return ;
}

quic_datagram_plaintext::~quic_datagram_plaintext() noexcept
{
    return ;
}

bool    networking_quic_enable_experimental() noexcept
{
    g_quic_experimental_enabled.store(true);
    ft_errno = FT_ERR_SUCCESSS;
    return (true);
}

bool    networking_quic_disable_experimental() noexcept
{
    g_quic_experimental_enabled.store(false);
    ft_errno = FT_ERR_SUCCESSS;
    return (true);
}

bool    networking_quic_is_experimental_enabled() noexcept
{
    bool enabled;

    enabled = g_quic_experimental_enabled.load();
    ft_errno = FT_ERR_SUCCESSS;
    return (enabled);
}

quic_experimental_session::quic_experimental_session() noexcept
{
    this->_ssl_session = ft_nullptr;
    this->_outbound = false;
    this->_configured = false;
    this->_send_sequence = 0;
    this->_receive_sequence = 0;
    this->_error_code = FT_ERR_SUCCESSS;
    this->_feature_configuration = quic_feature_configuration();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

quic_experimental_session::~quic_experimental_session() noexcept
{
    this->clear_key_material();
    this->_ssl_session = ft_nullptr;
    this->_configured = false;
    return ;
}

void    quic_experimental_session::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void    quic_experimental_session::clear_key_material() noexcept
{
    if (this->_send_key.size() > 0)
        ft_memset(this->_send_key.begin(), 0, this->_send_key.size());
    if (this->_receive_key.size() > 0)
        ft_memset(this->_receive_key.begin(), 0, this->_receive_key.size());
    if (this->_send_iv.size() > 0)
        ft_memset(this->_send_iv.begin(), 0, this->_send_iv.size());
    if (this->_receive_iv.size() > 0)
        ft_memset(this->_receive_iv.begin(), 0, this->_receive_iv.size());
    this->_send_key.clear();
    this->_receive_key.clear();
    this->_send_iv.clear();
    this->_receive_iv.clear();
    return ;
}

bool    quic_experimental_session::ensure_feature_enabled() const noexcept
{
    if (!networking_quic_is_experimental_enabled())
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (false);
    }
    return (true);
}

bool    quic_experimental_session::ensure_configured() const noexcept
{
    if (!this->_configured)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    return (true);
}

bool    quic_experimental_session::derive_keys(SSL *ssl_session, bool outbound) noexcept
{
    if (!networking_tls_export_aead_keys(ssl_session, outbound,
            this->_send_key, this->_send_iv, this->_receive_key, this->_receive_iv))
    {
        int export_error;

        export_error = ft_errno;
        this->set_error(export_error != FT_ERR_SUCCESSS ? export_error : FT_ERR_INTERNAL);
        return (false);
    }
    if (this->_send_key.size() == 0 || this->_receive_key.size() == 0)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    if (this->_send_iv.size() == 0 || this->_receive_iv.size() == 0)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    return (true);
}

bool    quic_experimental_session::prepare_nonce(uint64_t sequence_number,
        const ft_vector<unsigned char> &base_iv,
        ft_vector<unsigned char> &out_nonce) noexcept
{
    if (base_iv.size() == 0)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    out_nonce.resize(base_iv.size(), 0);
    if (out_nonce.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(out_nonce.get_error());
        return (false);
    }
    ft_vector<unsigned char> sequence_bytes;
    sequence_bytes.resize(base_iv.size(), 0);
    if (sequence_bytes.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(sequence_bytes.get_error());
        return (false);
    }
    size_t index;

    index = 0;
    while (index < base_iv.size())
    {
        size_t distance_from_end;

        distance_from_end = base_iv.size() - index - 1;
        if (distance_from_end < sizeof(uint64_t))
        {
            unsigned int bit_shift;
            unsigned char byte_value;

            bit_shift = static_cast<unsigned int>(distance_from_end * 8);
            byte_value = static_cast<unsigned char>((sequence_number >> bit_shift) & 0xFF);
            sequence_bytes[index] = byte_value;
        }
        else
            sequence_bytes[index] = 0;
        index++;
    }
    index = 0;
    while (index < base_iv.size())
    {
        out_nonce[index] = base_iv[index] ^ sequence_bytes[index];
        index++;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

bool    quic_experimental_session::configure(SSL *ssl_session,
        const quic_feature_configuration &configuration,
        bool outbound) noexcept
{
    if (!this->ensure_feature_enabled())
        return (false);
    if (ssl_session == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (SSL_is_init_finished(ssl_session) != 1)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    this->clear_key_material();
    if (!this->derive_keys(ssl_session, outbound))
        return (false);
    this->_ssl_session = ssl_session;
    this->_outbound = outbound;
    this->_configured = true;
    this->_feature_configuration = configuration;
    this->_send_sequence = 0;
    this->_receive_sequence = 0;
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

bool    quic_experimental_session::encrypt_datagram(const quic_datagram_plaintext &plaintext,
        ft_vector<unsigned char> &out_ciphertext) noexcept
{
    ft_vector<unsigned char> nonce;
    size_t payload_length;

    if (!this->ensure_feature_enabled())
        return (false);
    if (!this->ensure_configured())
        return (false);
    if (this->_send_sequence == UINT64_MAX)
    {
        this->set_error(FT_ERR_FULL);
        return (false);
    }
    payload_length = plaintext.payload_length;
    if (plaintext.payload_length > 0 && plaintext.payload == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (plaintext.associated_data_length > 0 && plaintext.associated_data == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (!this->prepare_nonce(this->_send_sequence, this->_send_iv, nonce))
        return (false);
    size_t ciphertext_size;

    ciphertext_size = payload_length + QUIC_EXPERIMENTAL_TAG_LENGTH;
    out_ciphertext.resize(ciphertext_size, 0);
    if (out_ciphertext.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(out_ciphertext.get_error());
        return (false);
    }
    unsigned char *ciphertext_pointer;
    unsigned char *tag_pointer;

    ciphertext_pointer = out_ciphertext.begin();
    tag_pointer = ciphertext_pointer;
    if (payload_length > 0)
        tag_pointer = ciphertext_pointer + payload_length;
    if (!encryption_aead_encrypt(this->_send_key.begin(), this->_send_key.size(),
            nonce.begin(), nonce.size(), plaintext.associated_data,
            plaintext.associated_data_length, plaintext.payload, payload_length,
            ciphertext_pointer, tag_pointer, QUIC_EXPERIMENTAL_TAG_LENGTH))
    {
        int encryption_error;

        encryption_error = ft_errno;
        this->set_error(encryption_error != FT_ERR_SUCCESSS ? encryption_error : FT_ERR_INTERNAL);
        return (false);
    }
    this->_send_sequence += 1;
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

bool    quic_experimental_session::decrypt_datagram(const ft_vector<unsigned char> &ciphertext,
        const unsigned char *associated_data,
        size_t associated_data_length,
        ft_vector<unsigned char> &out_plaintext) noexcept
{
    ft_vector<unsigned char> nonce;
    size_t ciphertext_length;
    size_t payload_length;

    if (!this->ensure_feature_enabled())
        return (false);
    if (!this->ensure_configured())
        return (false);
    if (this->_receive_sequence == UINT64_MAX)
    {
        this->set_error(FT_ERR_FULL);
        return (false);
    }
    ciphertext_length = ciphertext.size();
    if (ciphertext_length < QUIC_EXPERIMENTAL_TAG_LENGTH)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (associated_data_length > 0 && associated_data == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (!this->prepare_nonce(this->_receive_sequence, this->_receive_iv, nonce))
        return (false);
    payload_length = ciphertext_length - QUIC_EXPERIMENTAL_TAG_LENGTH;
    out_plaintext.resize(payload_length, 0);
    if (out_plaintext.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(out_plaintext.get_error());
        return (false);
    }
    const unsigned char *ciphertext_pointer;
    const unsigned char *tag_pointer;

    ciphertext_pointer = ciphertext.begin();
    tag_pointer = ciphertext_pointer + payload_length;
    if (!encryption_aead_decrypt(this->_receive_key.begin(), this->_receive_key.size(),
            nonce.begin(), nonce.size(), associated_data, associated_data_length,
            ciphertext_pointer, payload_length, tag_pointer,
            QUIC_EXPERIMENTAL_TAG_LENGTH, out_plaintext.begin()))
    {
        int decryption_error;

        decryption_error = ft_errno;
        this->set_error(decryption_error != FT_ERR_SUCCESSS ? decryption_error : FT_ERR_INTERNAL);
        return (false);
    }
    this->_receive_sequence += 1;
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

bool    quic_experimental_session::get_feature_configuration(quic_feature_configuration &out_configuration) const noexcept
{
    if (!this->ensure_configured())
        return (false);
    out_configuration = this->_feature_configuration;
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

int     quic_experimental_session::get_error() const noexcept
{
    return (this->_error_code);
}

const char  *quic_experimental_session::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

#endif
