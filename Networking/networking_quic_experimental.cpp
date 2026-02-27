#include "networking_quic_experimental.hpp"
#include "openssl_support.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

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
    return (true);
}

bool    networking_quic_disable_experimental() noexcept
{
    g_quic_experimental_enabled.store(false);
    return (true);
}

bool    networking_quic_is_experimental_enabled() noexcept
{
    bool enabled;

    enabled = g_quic_experimental_enabled.load();
    return (enabled);
}

quic_experimental_session::quic_experimental_session() noexcept
{
    this->_initialized_state = quic_experimental_session::_state_uninitialized;
    this->_ssl_session = ft_nullptr;
    this->_outbound = false;
    this->_configured = false;
    this->_send_sequence = 0;
    this->_receive_sequence = 0;
    this->_feature_configuration = quic_feature_configuration();
    return ;
}

quic_experimental_session::~quic_experimental_session() noexcept
{
    if (this->_initialized_state == quic_experimental_session::_state_initialized)
        (void)this->destroy();
    return ;
}

void    quic_experimental_session::abort_lifecycle_error(const char *method_name,
        const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "quic_experimental_session lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void    quic_experimental_session::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == quic_experimental_session::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name, "called while object is not initialized");
    return ;
}

int quic_experimental_session::initialize() noexcept
{
    if (this->_initialized_state == quic_experimental_session::_state_initialized)
        this->abort_lifecycle_error("quic_experimental_session::initialize",
            "initialize called on initialized instance");
    this->_ssl_session = ft_nullptr;
    this->_outbound = false;
    this->_configured = false;
    this->_send_sequence = 0;
    this->_receive_sequence = 0;
    this->_initialized_state = quic_experimental_session::_state_initialized;
    this->_feature_configuration = quic_feature_configuration();
    this->clear_key_material();
    return (FT_ERR_SUCCESS);
}

int quic_experimental_session::destroy() noexcept
{
    if (this->_initialized_state != quic_experimental_session::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->clear_key_material();
    this->_ssl_session = ft_nullptr;
    this->_outbound = false;
    this->_configured = false;
    this->_send_sequence = 0;
    this->_receive_sequence = 0;
    this->_initialized_state = quic_experimental_session::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

void    quic_experimental_session::clear_key_material() noexcept
{
    this->abort_if_not_initialized("quic_experimental_session::clear_key_material");
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
    this->abort_if_not_initialized("quic_experimental_session::ensure_feature_enabled");
    if (!networking_quic_is_experimental_enabled())
        return (false);
    return (true);
}

bool    quic_experimental_session::ensure_configured() const noexcept
{
    this->abort_if_not_initialized("quic_experimental_session::ensure_configured");
    if (!this->_configured)
        return (false);
    return (true);
}

bool    quic_experimental_session::derive_keys(SSL *ssl_session, bool outbound) noexcept
{
    this->abort_if_not_initialized("quic_experimental_session::derive_keys");
    if (!networking_tls_export_aead_keys(ssl_session, outbound,
            this->_send_key, this->_send_iv, this->_receive_key, this->_receive_iv))
        return (false);
    if (this->_send_key.size() == 0 || this->_receive_key.size() == 0)
        return (false);
    if (this->_send_iv.size() == 0 || this->_receive_iv.size() == 0)
        return (false);
    return (true);
}

bool    quic_experimental_session::prepare_nonce(uint64_t sequence_number,
        const ft_vector<unsigned char> &base_iv,
        ft_vector<unsigned char> &out_nonce) noexcept
{
    this->abort_if_not_initialized("quic_experimental_session::prepare_nonce");
    if (base_iv.size() == 0)
        return (false);
    out_nonce.resize(base_iv.size(), 0);
    ft_vector<unsigned char> sequence_bytes;
    sequence_bytes.resize(base_iv.size(), 0);
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
    return (true);
}

bool    quic_experimental_session::configure(SSL *ssl_session,
        const quic_feature_configuration &configuration,
        bool outbound) noexcept
{
    this->abort_if_not_initialized("quic_experimental_session::configure");
    if (!this->ensure_feature_enabled())
        return (false);
    if (ssl_session == ft_nullptr)
        return (false);
    if (SSL_is_init_finished(ssl_session) != 1)
        return (false);
    this->clear_key_material();
    if (!this->derive_keys(ssl_session, outbound))
        return (false);
    this->_ssl_session = ssl_session;
    this->_outbound = outbound;
    this->_configured = true;
    this->_feature_configuration = configuration;
    this->_send_sequence = 0;
    this->_receive_sequence = 0;
    return (true);
}

bool    quic_experimental_session::encrypt_datagram(const quic_datagram_plaintext &plaintext,
        ft_vector<unsigned char> &out_ciphertext) noexcept
{
    ft_vector<unsigned char> nonce;
    size_t payload_length;

    this->abort_if_not_initialized("quic_experimental_session::encrypt_datagram");
    if (!this->ensure_feature_enabled())
        return (false);
    if (!this->ensure_configured())
        return (false);
    if (this->_send_sequence == UINT64_MAX)
        return (false);
    payload_length = plaintext.payload_length;
    if (plaintext.payload_length > 0 && plaintext.payload == ft_nullptr)
        return (false);
    if (plaintext.associated_data_length > 0 && plaintext.associated_data == ft_nullptr)
        return (false);
    if (!this->prepare_nonce(this->_send_sequence, this->_send_iv, nonce))
        return (false);
    size_t ciphertext_size;

    ciphertext_size = payload_length + QUIC_EXPERIMENTAL_TAG_LENGTH;
    out_ciphertext.resize(ciphertext_size, 0);
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
        return (false);
    this->_send_sequence += 1;
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

    this->abort_if_not_initialized("quic_experimental_session::decrypt_datagram");
    if (!this->ensure_feature_enabled())
        return (false);
    if (!this->ensure_configured())
        return (false);
    if (this->_receive_sequence == UINT64_MAX)
        return (false);
    ciphertext_length = ciphertext.size();
    if (ciphertext_length < QUIC_EXPERIMENTAL_TAG_LENGTH)
        return (false);
    if (associated_data_length > 0 && associated_data == ft_nullptr)
        return (false);
    if (!this->prepare_nonce(this->_receive_sequence, this->_receive_iv, nonce))
        return (false);
    payload_length = ciphertext_length - QUIC_EXPERIMENTAL_TAG_LENGTH;
    out_plaintext.resize(payload_length, 0);
    const unsigned char *ciphertext_pointer;
    const unsigned char *tag_pointer;

    ciphertext_pointer = ciphertext.begin();
    tag_pointer = ciphertext_pointer + payload_length;
    if (!encryption_aead_decrypt(this->_receive_key.begin(), this->_receive_key.size(),
            nonce.begin(), nonce.size(), associated_data, associated_data_length,
            ciphertext_pointer, payload_length, tag_pointer,
            QUIC_EXPERIMENTAL_TAG_LENGTH, out_plaintext.begin()))
        return (false);
    this->_receive_sequence += 1;
    return (true);
}

bool    quic_experimental_session::get_feature_configuration(quic_feature_configuration &out_configuration) const noexcept
{
    this->abort_if_not_initialized("quic_experimental_session::get_feature_configuration");
    if (!this->ensure_configured())
        return (false);
    out_configuration = this->_feature_configuration;
    return (true);
}

#endif
