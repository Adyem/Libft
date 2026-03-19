#include <stdint.h>
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "encryption.hpp"

static uint32_t left_rotate(uint32_t value, uint32_t bits)
{
    return ((value << bits) | (value >> (32 - bits)));
}

void sha1_hash(const void *data, ft_size_t length, uint8_t *digest)
{
    uint32_t hash_values[5];
    uint64_t bit_length;
    ft_size_t padded_length;
    uint8_t *message;
    const uint8_t *byte_data;
    ft_size_t copy_index;
    ft_size_t length_index;
    ft_size_t chunk_offset;

    if (!digest)
        return ;
    copy_index = 0;
    while (copy_index < 20)
    {
        digest[copy_index] = 0;
        copy_index++;
    }

    hash_values[0] = 0x67452301;
    hash_values[1] = 0xEFCDAB89;
    hash_values[2] = 0x98BADCFE;
    hash_values[3] = 0x10325476;
    hash_values[4] = 0xC3D2E1F0;
    bit_length = length * 8;
    padded_length = length + 1;
    while ((padded_length % 64) != 56)
    {
        padded_length++;
    }
    message = static_cast<uint8_t *>(cma_malloc(padded_length + 8));
    if (!message)
        return ;
    byte_data = static_cast<const uint8_t *>(data);
    copy_index = 0;
    while (copy_index < length)
    {
        message[copy_index] = byte_data[copy_index];
        copy_index++;
    }
    message[length] = 0x80;
    copy_index = length + 1;
    while (copy_index < padded_length)
    {
        message[copy_index] = 0;
        copy_index++;
    }
    length_index = 0;
    while (length_index < 8)
    {
        message[padded_length + length_index] = static_cast<uint8_t>(
                (bit_length >> ((7 - length_index) * 8)) & 0xFF);
        length_index++;
    }
    chunk_offset = 0;
    while (chunk_offset < padded_length + 8)
    {
        uint32_t words[80];
        ft_size_t word_index;

        word_index = 0;
        while (word_index < 16)
        {
            ft_size_t byte_index;

            byte_index = chunk_offset + word_index * 4;
            words[word_index] = static_cast<uint32_t>(message[byte_index]) << 24;
            words[word_index] |= static_cast<uint32_t>(message[byte_index + 1]) << 16;
            words[word_index] |= static_cast<uint32_t>(message[byte_index + 2]) << 8;
            words[word_index] |= static_cast<uint32_t>(message[byte_index + 3]);
            word_index++;
        }
        while (word_index < 80)
        {
            uint32_t value;

            value = words[word_index - 3] ^ words[word_index - 8] ^ words[word_index - 14] ^ words[word_index - 16];
            words[word_index] = left_rotate(value, 1);
            word_index++;
        }
        uint32_t hash_working_value_a;
        uint32_t hash_working_value_b;
        uint32_t hash_working_value_c;
        uint32_t hash_working_value_d;
        uint32_t hash_working_value_e;

        hash_working_value_a = hash_values[0];
        hash_working_value_b = hash_values[1];
        hash_working_value_c = hash_values[2];
        hash_working_value_d = hash_values[3];
        hash_working_value_e = hash_values[4];
        word_index = 0;
        while (word_index < 80)
        {
            uint32_t function_value;
            uint32_t constant_value;
            uint32_t temp_value;

            if (word_index < 20)
            {
                function_value = (hash_working_value_b & hash_working_value_c)
                    | ((~hash_working_value_b) & hash_working_value_d);
                constant_value = 0x5A827999;
            }
            else if (word_index < 40)
            {
                function_value = hash_working_value_b
                    ^ hash_working_value_c ^ hash_working_value_d;
                constant_value = 0x6ED9EBA1;
            }
            else if (word_index < 60)
            {
                function_value = (hash_working_value_b
                        & hash_working_value_c)
                    | (hash_working_value_b & hash_working_value_d)
                    | (hash_working_value_c & hash_working_value_d);
                constant_value = 0x8F1BBCDC;
            }
            else
            {
                function_value = hash_working_value_b
                    ^ hash_working_value_c ^ hash_working_value_d;
                constant_value = 0xCA62C1D6;
            }
            temp_value = left_rotate(hash_working_value_a, 5)
                + function_value + hash_working_value_e + constant_value
                + words[word_index];
            hash_working_value_e = hash_working_value_d;
            hash_working_value_d = hash_working_value_c;
            hash_working_value_c = left_rotate(hash_working_value_b, 30);
            hash_working_value_b = hash_working_value_a;
            hash_working_value_a = temp_value;
            word_index++;
        }
        hash_values[0] += hash_working_value_a;
        hash_values[1] += hash_working_value_b;
        hash_values[2] += hash_working_value_c;
        hash_values[3] += hash_working_value_d;
        hash_values[4] += hash_working_value_e;
        chunk_offset += 64;
    }
    length_index = 0;
    while (length_index < 5)
    {
        digest[length_index * 4] = static_cast<uint8_t>(
                (hash_values[length_index] >> 24) & 0xFF);
        digest[length_index * 4 + 1] = static_cast<uint8_t>(
                (hash_values[length_index] >> 16) & 0xFF);
        digest[length_index * 4 + 2] = static_cast<uint8_t>(
                (hash_values[length_index] >> 8) & 0xFF);
        digest[length_index * 4 + 3] = static_cast<uint8_t>(
                hash_values[length_index] & 0xFF);
        length_index++;
    }
    cma_free(message);
    return ;
}
