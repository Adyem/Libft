#include <stdint.h>
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "encryption.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

static uint32_t rotate_right(uint32_t value, uint32_t bits)
{
    return ((value >> bits) | (value << (32 - bits)));
}

static const uint32_t sha256_constants[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

void sha256_hash(const void *data, ft_size_t length, uint8_t *digest)
{
    uint32_t hash_values[8] = {
        0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
        0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19
    };
    uint64_t bit_length = length * 8;
    ft_size_t padded_length = length + 1;
    ft_size_t digest_index;

    if (!digest)
        return ;
    digest_index = 0;
    while (digest_index < 32)
    {
        digest[digest_index] = 0;
        digest_index++;
    }
    while (padded_length % 64 != 56)
    {
        ++padded_length;
    }
    uint8_t *message = static_cast<uint8_t *>(cma_malloc(padded_length + 8));
    if (!message)
        return ;
    ft_size_t copy_index = 0;
    const uint8_t *byte_data = static_cast<const uint8_t *>(data);
    while (copy_index < length)
    {
        message[copy_index] = byte_data[copy_index];
        ++copy_index;
    }
    message[length] = 0x80;
    copy_index = length + 1;
    while (copy_index < padded_length)
    {
        message[copy_index] = 0;
        ++copy_index;
    }
    message[padded_length] = static_cast<uint8_t>((bit_length >> 56) & 0xFF);
    message[padded_length + 1] = static_cast<uint8_t>((bit_length >> 48) & 0xFF);
    message[padded_length + 2] = static_cast<uint8_t>((bit_length >> 40) & 0xFF);
    message[padded_length + 3] = static_cast<uint8_t>((bit_length >> 32) & 0xFF);
    message[padded_length + 4] = static_cast<uint8_t>((bit_length >> 24) & 0xFF);
    message[padded_length + 5] = static_cast<uint8_t>((bit_length >> 16) & 0xFF);
    message[padded_length + 6] = static_cast<uint8_t>((bit_length >> 8) & 0xFF);
    message[padded_length + 7] = static_cast<uint8_t>(bit_length & 0xFF);
    ft_size_t chunk_offset = 0;
    while (chunk_offset < padded_length + 8)
    {
        uint32_t words[64];
        ft_size_t word_index = 0;
        while (word_index < 16)
        {
            ft_size_t byte_index = chunk_offset + word_index * 4;
            words[word_index] = (static_cast<uint32_t>(message[byte_index]) << 24) |
                (static_cast<uint32_t>(message[byte_index + 1]) << 16) |
                (static_cast<uint32_t>(message[byte_index + 2]) << 8) |
                static_cast<uint32_t>(message[byte_index + 3]);
            ++word_index;
        }
        while (word_index < 64)
        {
            uint32_t sigma_zero = rotate_right(words[word_index - 15], 7) ^ rotate_right(words[word_index - 15], 18) ^ (words[word_index - 15] >> 3);
            uint32_t sigma_one = rotate_right(words[word_index - 2], 17) ^ rotate_right(words[word_index - 2], 19) ^ (words[word_index - 2] >> 10);
            words[word_index] = words[word_index - 16] + sigma_zero + words[word_index - 7] + sigma_one;
            ++word_index;
        }
        uint32_t hash_working_value_a = hash_values[0];
        uint32_t hash_working_value_b = hash_values[1];
        uint32_t hash_working_value_c = hash_values[2];
        uint32_t hash_working_value_d = hash_values[3];
        uint32_t hash_working_value_e = hash_values[4];
        uint32_t hash_working_value_f = hash_values[5];
        uint32_t hash_working_value_g = hash_values[6];
        uint32_t hash_working_value_h = hash_values[7];
        word_index = 0;
        while (word_index < 64)
        {
            uint32_t big_sigma_one = rotate_right(hash_working_value_e, 6)
                ^ rotate_right(hash_working_value_e, 11)
                ^ rotate_right(hash_working_value_e, 25);
            uint32_t choose = (hash_working_value_e & hash_working_value_f)
                ^ ((~hash_working_value_e) & hash_working_value_g);
            uint32_t temporary_sum_one = hash_working_value_h + big_sigma_one + choose
                + sha256_constants[word_index] + words[word_index];
            uint32_t big_sigma_zero = rotate_right(hash_working_value_a, 2)
                ^ rotate_right(hash_working_value_a, 13)
                ^ rotate_right(hash_working_value_a, 22);
            uint32_t majority = (hash_working_value_a & hash_working_value_b)
                ^ (hash_working_value_a & hash_working_value_c)
                ^ (hash_working_value_b & hash_working_value_c);
            uint32_t temporary_sum_two = big_sigma_zero + majority;
            hash_working_value_h = hash_working_value_g;
            hash_working_value_g = hash_working_value_f;
            hash_working_value_f = hash_working_value_e;
            hash_working_value_e = hash_working_value_d + temporary_sum_one;
            hash_working_value_d = hash_working_value_c;
            hash_working_value_c = hash_working_value_b;
            hash_working_value_b = hash_working_value_a;
            hash_working_value_a = temporary_sum_one + temporary_sum_two;
            ++word_index;
        }
        hash_values[0] += hash_working_value_a;
        hash_values[1] += hash_working_value_b;
        hash_values[2] += hash_working_value_c;
        hash_values[3] += hash_working_value_d;
        hash_values[4] += hash_working_value_e;
        hash_values[5] += hash_working_value_f;
        hash_values[6] += hash_working_value_g;
        hash_values[7] += hash_working_value_h;
        chunk_offset += 64;
    }
    digest_index = 0;
    while (digest_index < 8)
    {
        digest[digest_index * 4] = static_cast<uint8_t>((hash_values[digest_index] >> 24) & 0xFF);
        digest[digest_index * 4 + 1] = static_cast<uint8_t>((hash_values[digest_index] >> 16) & 0xFF);
        digest[digest_index * 4 + 2] = static_cast<uint8_t>((hash_values[digest_index] >> 8) & 0xFF);
        digest[digest_index * 4 + 3] = static_cast<uint8_t>(hash_values[digest_index] & 0xFF);
        ++digest_index;
    }
    cma_free(message);
    return ;
}
