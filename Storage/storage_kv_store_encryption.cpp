#include "kv_store.hpp"

#include "../CMA/CMA.hpp"
#include "../Compression/compression.hpp"
#include "../Encryption/aes.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

int kv_store::encrypt_value(const ft_string &plain_string, ft_string &encoded_string) const
{
    std::vector<uint8_t> output_buffer;
    const char *plain_c_string;
    std::size_t plain_size;
    uint8_t header_plain[16];
    uint8_t header_cipher[16];
    std::size_t header_index;
    uint8_t counter_block[16];
    std::size_t counter_index;
    unsigned char *encoded_buffer;
    std::size_t encoded_size;
    std::size_t output_index;
    std::size_t byte_index;
    uint64_t block_counter;

    if (this->_encryption_enabled == false)
    {
        encoded_string = plain_string;
        if (encoded_string.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(encoded_string.get_error());
            return (-1);
        }
        return (0);
    }
    if (this->_encryption_key.size() != 16)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    plain_c_string = plain_string.c_str();
    plain_size = plain_string.size();
    header_index = 0;
    while (header_index < 16)
    {
        header_plain[header_index] = static_cast<uint8_t>("kvstore-aes-ctr!"[header_index]);
        header_cipher[header_index] = header_plain[header_index];
        header_index++;
    }
    aes_encrypt(header_cipher, reinterpret_cast<const uint8_t *>(this->_encryption_key.c_str()));
    output_buffer.reserve(16 + plain_size);
    header_index = 0;
    while (header_index < 16)
    {
        output_buffer.push_back(header_cipher[header_index]);
        header_index++;
    }
    block_counter = 0;
    output_index = 0;
    while (output_index < plain_size)
    {
        counter_index = 0;
        while (counter_index < 16)
        {
            counter_block[counter_index] = header_plain[counter_index];
            counter_index++;
        }
        counter_index = 0;
        while (counter_index < 8)
        {
            counter_block[15 - counter_index] = static_cast<uint8_t>((block_counter >> (counter_index * 8)) & 0xFF);
            counter_index++;
        }
        aes_encrypt(counter_block, reinterpret_cast<const uint8_t *>(this->_encryption_key.c_str()));
        byte_index = 0;
        while (byte_index < 16 && output_index < plain_size)
        {
            uint8_t cipher_byte;

            cipher_byte = static_cast<uint8_t>(static_cast<unsigned char>(plain_c_string[output_index]) ^ counter_block[byte_index]);
            output_buffer.push_back(cipher_byte);
            output_index++;
            byte_index++;
        }
        block_counter++;
    }
    encoded_size = 0;
    encoded_buffer = ft_base64_encode(reinterpret_cast<const unsigned char *>(output_buffer.data()), output_buffer.size(), &encoded_size);
    if (encoded_buffer == ft_nullptr)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    encoded_string.clear();
    if (encoded_string.get_error() != FT_ER_SUCCESSS)
    {
        cma_free(encoded_buffer);
        this->set_error(encoded_string.get_error());
        return (-1);
    }
    output_index = 0;
    while (output_index < encoded_size)
    {
        encoded_string.append(static_cast<char>(encoded_buffer[output_index]));
        if (encoded_string.get_error() != FT_ER_SUCCESSS)
        {
            cma_free(encoded_buffer);
            this->set_error(encoded_string.get_error());
            return (-1);
        }
        output_index++;
    }
    cma_free(encoded_buffer);
    return (0);
}

int kv_store::decrypt_value(const ft_string &encoded_string, ft_string &plain_string) const
{
    unsigned char *decoded_buffer;
    std::size_t decoded_size;
    uint8_t header_plain[16];
    uint8_t expected_header[16];
    std::size_t header_index;
    std::size_t payload_size;
    const unsigned char *payload_pointer;
    uint8_t counter_block[16];
    std::size_t counter_index;
    std::size_t payload_index;
    uint64_t block_counter;
    std::size_t byte_index;

    if (this->_encryption_enabled == false)
    {
        plain_string = encoded_string;
        if (plain_string.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(plain_string.get_error());
            return (-1);
        }
        return (0);
    }
    if (this->_encryption_key.size() != 16)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    decoded_size = 0;
    decoded_buffer = ft_base64_decode(reinterpret_cast<const unsigned char *>(encoded_string.c_str()), encoded_string.size(), &decoded_size);
    if (decoded_buffer == ft_nullptr)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    if (decoded_size == 0)
    {
        plain_string.clear();
        if (plain_string.get_error() != FT_ER_SUCCESSS)
        {
            cma_free(decoded_buffer);
            this->set_error(plain_string.get_error());
            return (-1);
        }
        cma_free(decoded_buffer);
        return (0);
    }
    if (decoded_size < 16)
    {
        cma_free(decoded_buffer);
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    header_index = 0;
    while (header_index < 16)
    {
        header_plain[header_index] = static_cast<uint8_t>("kvstore-aes-ctr!"[header_index]);
        expected_header[header_index] = header_plain[header_index];
        header_index++;
    }
    aes_encrypt(expected_header, reinterpret_cast<const uint8_t *>(this->_encryption_key.c_str()));
    header_index = 0;
    while (header_index < 16)
    {
        if (decoded_buffer[header_index] != expected_header[header_index])
        {
            cma_free(decoded_buffer);
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        header_index++;
    }
    payload_size = decoded_size - 16;
    payload_pointer = decoded_buffer + 16;
    plain_string.clear();
    if (plain_string.get_error() != FT_ER_SUCCESSS)
    {
        cma_free(decoded_buffer);
        this->set_error(plain_string.get_error());
        return (-1);
    }
    payload_index = 0;
    block_counter = 0;
    while (payload_index < payload_size)
    {
        counter_index = 0;
        while (counter_index < 16)
        {
            counter_block[counter_index] = header_plain[counter_index];
            counter_index++;
        }
        counter_index = 0;
        while (counter_index < 8)
        {
            counter_block[15 - counter_index] = static_cast<uint8_t>((block_counter >> (counter_index * 8)) & 0xFF);
            counter_index++;
        }
        aes_encrypt(counter_block, reinterpret_cast<const uint8_t *>(this->_encryption_key.c_str()));
        byte_index = 0;
        while (byte_index < 16 && payload_index < payload_size)
        {
            char plain_char;

            plain_char = static_cast<char>(payload_pointer[payload_index] ^ counter_block[byte_index]);
            plain_string.append(plain_char);
            if (plain_string.get_error() != FT_ER_SUCCESSS)
            {
                cma_free(decoded_buffer);
                this->set_error(plain_string.get_error());
                return (-1);
            }
            payload_index++;
            byte_index++;
        }
        block_counter++;
    }
    cma_free(decoded_buffer);
    return (0);
}

int kv_store::configure_encryption(const char *encryption_key, bool enable_encryption)
{
    if (enable_encryption)
    {
        if (encryption_key == ft_nullptr)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        this->_encryption_key = encryption_key;
        if (this->_encryption_key.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(this->_encryption_key.get_error());
            return (-1);
        }
        if (this->_encryption_key.size() != 16)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        this->_encryption_enabled = true;
        this->set_error(FT_ER_SUCCESSS);
        return (0);
    }
    this->_encryption_enabled = false;
    if (encryption_key != ft_nullptr)
    {
        this->_encryption_key = encryption_key;
        if (this->_encryption_key.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(this->_encryption_key.get_error());
            return (-1);
        }
    }
    else
        this->_encryption_key.clear();
    this->set_error(FT_ER_SUCCESSS);
    return (0);
}

