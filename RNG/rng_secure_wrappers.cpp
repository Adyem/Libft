#include "rng.hpp"
#include "../Errno/errno.hpp"
#include <random>
#include <exception>

static int rng_fill_from_random_device(unsigned char *buffer, size_t length)
{
    try
    {
        std::random_device random_device;
        size_t offset = 0;
        while (offset < length)
        {
            unsigned int random_value = random_device();
            size_t byte_index = 0;
            while (byte_index < sizeof(unsigned int) && offset < length)
            {
                unsigned int shifted_value = random_value >> (byte_index * 8);
                buffer[offset] = static_cast<unsigned char>(shifted_value & 0xFFu);
                offset++;
                byte_index++;
            }
        }
    }
    catch (const std::exception &)
    {
        ft_errno = FT_ERR_INTERNAL;
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

int rng_secure_bytes_with_fallback(unsigned char *buffer, size_t length, int *fallback_used)
{
    if (buffer == ft_nullptr && length > 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        if (fallback_used != ft_nullptr)
            *fallback_used = 0;
        return (-1);
    }
    if (fallback_used != ft_nullptr)
        *fallback_used = 0;
    int result = rng_secure_bytes(buffer, length);
    if (result == 0)
        return (0);
    if (length == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    int fallback_result = rng_fill_from_random_device(buffer, length);
    if (fallback_result != 0)
        return (-1);
    if (fallback_used != ft_nullptr)
        *fallback_used = 1;
    ft_errno = ER_SUCCESS;
    return (0);
}

int rng_secure_uint64(uint64_t *value, int *fallback_used)
{
    if (value == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        if (fallback_used != ft_nullptr)
            *fallback_used = 0;
        return (-1);
    }
    unsigned char buffer[sizeof(uint64_t)];
    int result = rng_secure_bytes_with_fallback(buffer, sizeof(uint64_t), fallback_used);
    if (result != 0)
        return (-1);
    size_t index = 0;
    uint64_t assembled_value = 0;
    while (index < sizeof(uint64_t))
    {
        uint64_t shifted_value = static_cast<uint64_t>(buffer[index]);
        shifted_value <<= (index * 8);
        assembled_value |= shifted_value;
        index++;
    }
    *value = assembled_value;
    ft_errno = ER_SUCCESS;
    return (0);
}

int rng_secure_uint32(uint32_t *value, int *fallback_used)
{
    if (value == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        if (fallback_used != ft_nullptr)
            *fallback_used = 0;
        return (-1);
    }
    uint64_t wide_value = 0;
    int result = rng_secure_uint64(&wide_value, fallback_used);
    if (result != 0)
        return (-1);
    *value = static_cast<uint32_t>(wide_value & 0xFFFFFFFFu);
    ft_errno = ER_SUCCESS;
    return (0);
}
