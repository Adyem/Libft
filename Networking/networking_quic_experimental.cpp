#include "networking_quic_experimental.hpp"
#include "openssl_support.hpp"
#include "../Errno/errno_internal.hpp"

#if NETWORKING_HAS_OPENSSL

#include <atomic>

static std::atomic<ft_bool> g_quic_experimental_enabled(FT_FALSE);

static const ft_size_t QUIC_EXPERIMENTAL_TAG_LENGTH = 16;

quic_feature_configuration::quic_feature_configuration() noexcept
{
    this->enable_datagram_pacing = FT_FALSE;
    this->enable_loss_recovery = FT_FALSE;
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

ft_bool    networking_quic_enable_experimental() noexcept
{
    g_quic_experimental_enabled.store(FT_TRUE);
    return (FT_TRUE);
}

ft_bool    networking_quic_disable_experimental() noexcept
{
    g_quic_experimental_enabled.store(FT_FALSE);
    return (FT_TRUE);
}

ft_bool    networking_quic_is_experimental_enabled() noexcept
{
    ft_bool enabled;

    enabled = g_quic_experimental_enabled.load();
    return (enabled);
}

quic_experimental_session::quic_experimental_session() noexcept
{
    this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    this->_ssl_session = ft_nullptr;
    this->_outbound = FT_FALSE;
    this->_configured = FT_FALSE;
    this->_send_sequence = 0;
    this->_receive_sequence = 0;
    this->_feature_configuration = quic_feature_configuration();
    return ;
}

quic_experimental_session::~quic_experimental_session() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

quic_experimental_session::quic_experimental_session(const quic_experimental_session &other) noexcept
    : quic_experimental_session()
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "quic_experimental_session::quic_experimental_session(copy)",
            "source is uninitialised");
    if (this->initialize(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

quic_experimental_session::quic_experimental_session(quic_experimental_session &&other) noexcept
    : quic_experimental_session()
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "quic_experimental_session::quic_experimental_session(move)",
            "source is uninitialised");
    if (this->initialize(static_cast<quic_experimental_session &&>(other)) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

int32_t quic_experimental_session::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "quic_experimental_session::initialize", "initialize called on initialised instance");
    this->_ssl_session = ft_nullptr;
    this->_outbound = FT_FALSE;
    this->_configured = FT_FALSE;
    this->_send_sequence = 0;
    this->_receive_sequence = 0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->_feature_configuration = quic_feature_configuration();
    this->clear_key_material();
    return (FT_ERR_SUCCESS);
}

int32_t quic_experimental_session::destroy() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    this->clear_key_material();
    this->_ssl_session = ft_nullptr;
    this->_outbound = FT_FALSE;
    this->_configured = FT_FALSE;
    this->_send_sequence = 0;
    this->_receive_sequence = 0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t quic_experimental_session::move(quic_experimental_session &other) noexcept
{
    return (this->initialize(static_cast<quic_experimental_session &&>(other)));
}

int32_t quic_experimental_session::initialize(const quic_experimental_session &other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "quic_experimental_session::initialize(const quic_experimental_session &)",
            "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    return (this->initialize());
}

int32_t quic_experimental_session::initialize(quic_experimental_session &&other) noexcept
{
    int32_t initialize_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize(other);
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    (void)other.destroy();
    return (FT_ERR_SUCCESS);
}

void    quic_experimental_session::clear_key_material() noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "quic_experimental_session::clear_key_material");
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

ft_bool    quic_experimental_session::ensure_feature_enabled() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "quic_experimental_session::ensure_feature_enabled");
    if (!networking_quic_is_experimental_enabled())
        return (FT_FALSE);
    return (FT_TRUE);
}

ft_bool    quic_experimental_session::ensure_configured() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "quic_experimental_session::ensure_configured");
    if (!this->_configured)
        return (FT_FALSE);
    return (FT_TRUE);
}

ft_bool    quic_experimental_session::derive_keys(SSL *ssl_session, ft_bool outbound) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "quic_experimental_session::derive_keys");
    if (!networking_tls_export_aead_keys(ssl_session, outbound,
            this->_send_key, this->_send_iv, this->_receive_key, this->_receive_iv))
        return (FT_FALSE);
    if (this->_send_key.size() == 0 || this->_receive_key.size() == 0)
        return (FT_FALSE);
    if (this->_send_iv.size() == 0 || this->_receive_iv.size() == 0)
        return (FT_FALSE);
    return (FT_TRUE);
}

ft_bool    quic_experimental_session::prepare_nonce(uint64_t sequence_number,
        const ft_vector<unsigned char> &base_iv,
        ft_vector<unsigned char> &out_nonce) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "quic_experimental_session::prepare_nonce");
    if (base_iv.size() == 0)
        return (FT_FALSE);
    out_nonce.resize(base_iv.size(), 0);
    ft_vector<unsigned char> sequence_bytes;
    sequence_bytes.resize(base_iv.size(), 0);
    ft_size_t index;

    index = 0;
    while (index < base_iv.size())
    {
        ft_size_t distance_from_end;

        distance_from_end = base_iv.size() - index - 1;
        if (distance_from_end < sizeof(uint64_t))
        {
            uint32_t bit_shift;
            unsigned char byte_value;

            bit_shift = static_cast<uint32_t>(distance_from_end * 8);
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
    return (FT_TRUE);
}

ft_bool    quic_experimental_session::configure(SSL *ssl_session,
        const quic_feature_configuration &configuration,
        ft_bool outbound) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "quic_experimental_session::configure");
    if (!this->ensure_feature_enabled())
        return (FT_FALSE);
    if (ssl_session == ft_nullptr)
        return (FT_FALSE);
    if (SSL_is_init_finished(ssl_session) != 1)
        return (FT_FALSE);
    this->clear_key_material();
    if (!this->derive_keys(ssl_session, outbound))
        return (FT_FALSE);
    this->_ssl_session = ssl_session;
    this->_outbound = outbound;
    this->_configured = FT_TRUE;
    this->_feature_configuration = configuration;
    this->_send_sequence = 0;
    this->_receive_sequence = 0;
    return (FT_TRUE);
}

ft_bool    quic_experimental_session::encrypt_datagram(const quic_datagram_plaintext &plaintext,
        ft_vector<unsigned char> &out_ciphertext) noexcept
{
    ft_vector<unsigned char> nonce;
    ft_size_t payload_length;

    errno_abort_if_uninitialised(this->_initialised_state, "quic_experimental_session::encrypt_datagram");
    if (!this->ensure_feature_enabled())
        return (FT_FALSE);
    if (!this->ensure_configured())
        return (FT_FALSE);
    if (this->_send_sequence == UINT64_MAX)
        return (FT_FALSE);
    payload_length = plaintext.payload_length;
    if (plaintext.payload_length > 0 && plaintext.payload == ft_nullptr)
        return (FT_FALSE);
    if (plaintext.associated_data_length > 0 && plaintext.associated_data == ft_nullptr)
        return (FT_FALSE);
    if (!this->prepare_nonce(this->_send_sequence, this->_send_iv, nonce))
        return (FT_FALSE);
    ft_size_t ciphertext_size;

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
        return (FT_FALSE);
    this->_send_sequence += 1;
    return (FT_TRUE);
}

ft_bool    quic_experimental_session::decrypt_datagram(const ft_vector<unsigned char> &ciphertext,
        const unsigned char *associated_data,
        ft_size_t associated_data_length,
        ft_vector<unsigned char> &out_plaintext) noexcept
{
    ft_vector<unsigned char> nonce;
    ft_size_t ciphertext_length;
    ft_size_t payload_length;

    errno_abort_if_uninitialised(this->_initialised_state, "quic_experimental_session::decrypt_datagram");
    if (!this->ensure_feature_enabled())
        return (FT_FALSE);
    if (!this->ensure_configured())
        return (FT_FALSE);
    if (this->_receive_sequence == UINT64_MAX)
        return (FT_FALSE);
    ciphertext_length = ciphertext.size();
    if (ciphertext_length < QUIC_EXPERIMENTAL_TAG_LENGTH)
        return (FT_FALSE);
    if (associated_data_length > 0 && associated_data == ft_nullptr)
        return (FT_FALSE);
    if (!this->prepare_nonce(this->_receive_sequence, this->_receive_iv, nonce))
        return (FT_FALSE);
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
        return (FT_FALSE);
    this->_receive_sequence += 1;
    return (FT_TRUE);
}

ft_bool    quic_experimental_session::get_feature_configuration(quic_feature_configuration &out_configuration) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "quic_experimental_session::get_feature_configuration");
    if (!this->ensure_configured())
        return (FT_FALSE);
    out_configuration = this->_feature_configuration;
    return (FT_TRUE);
}

#endif
