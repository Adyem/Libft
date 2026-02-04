#include <stdint.h>
#include <stddef.h>
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "encryption_sha1.hpp"

static uint32_t left_rotate(uint32_t value, uint32_t bits)
{
    return ((value << bits) | (value >> (32 - bits)));
}

void sha1_hash(const void *data, size_t length, unsigned char *digest)
{
    uint32_t hash_values[5];
    uint64_t bit_length;
    size_t padded_length;
    unsigned char *message;
    const unsigned char *byte_data;
    size_t copy_index;
    size_t length_index;
    size_t chunk_offset;

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
    message = static_cast<unsigned char *>(cma_malloc(padded_length + 8));
    if (!message)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return ;
    }
    byte_data = static_cast<const unsigned char *>(data);
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
        message[padded_length + length_index] = static_cast<unsigned char>((bit_length >> ((7 - length_index) * 8)) & 0xFF);
        length_index++;
    }
    chunk_offset = 0;
    while (chunk_offset < padded_length + 8)
    {
        uint32_t words[80];
        size_t word_index;

        word_index = 0;
        while (word_index < 16)
        {
            size_t byte_index;

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
        uint32_t hash_a;
        uint32_t hash_b;
        uint32_t hash_c;
        uint32_t hash_d;
        uint32_t hash_e;

        hash_a = hash_values[0];
        hash_b = hash_values[1];
        hash_c = hash_values[2];
        hash_d = hash_values[3];
        hash_e = hash_values[4];
        word_index = 0;
        while (word_index < 80)
        {
            uint32_t function_value;
            uint32_t constant_value;
            uint32_t temp_value;

            if (word_index < 20)
            {
                function_value = (hash_b & hash_c) | ((~hash_b) & hash_d);
                constant_value = 0x5A827999;
            }
            else if (word_index < 40)
            {
                function_value = hash_b ^ hash_c ^ hash_d;
                constant_value = 0x6ED9EBA1;
            }
            else if (word_index < 60)
            {
                function_value = (hash_b & hash_c) | (hash_b & hash_d) | (hash_c & hash_d);
                constant_value = 0x8F1BBCDC;
            }
            else
            {
                function_value = hash_b ^ hash_c ^ hash_d;
                constant_value = 0xCA62C1D6;
            }
            temp_value = left_rotate(hash_a, 5) + function_value + hash_e + constant_value + words[word_index];
            hash_e = hash_d;
            hash_d = hash_c;
            hash_c = left_rotate(hash_b, 30);
            hash_b = hash_a;
            hash_a = temp_value;
            word_index++;
        }
        hash_values[0] += hash_a;
        hash_values[1] += hash_b;
        hash_values[2] += hash_c;
        hash_values[3] += hash_d;
        hash_values[4] += hash_e;
        chunk_offset += 64;
    }
    length_index = 0;
    while (length_index < 5)
    {
        digest[length_index * 4] = static_cast<unsigned char>((hash_values[length_index] >> 24) & 0xFF);
        digest[length_index * 4 + 1] = static_cast<unsigned char>((hash_values[length_index] >> 16) & 0xFF);
        digest[length_index * 4 + 2] = static_cast<unsigned char>((hash_values[length_index] >> 8) & 0xFF);
        digest[length_index * 4 + 3] = static_cast<unsigned char>(hash_values[length_index] & 0xFF);
        length_index++;
    }
    cma_free(message);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
