#include "rng.hpp"
#include "../Errno/errno.hpp"
#include <random>
#include <exception>

static int32_t rng_fill_from_random_device(unsigned char *buffer, ft_size_t length)
{
    ft_size_t offset;

    try
    {
        std::random_device random_device;
        offset = 0;
        while (offset < length)
        {
            uint32_t random_value = random_device();
            ft_size_t byte_index = 0;
            while (byte_index < sizeof(uint32_t) && offset < length)
            {
                uint32_t shifted_value = random_value >> (byte_index * 8);
                buffer[offset] = static_cast<unsigned char>(shifted_value & 0xFFu);
                offset++;
                byte_index++;
            }
        }
    }
    catch (const std::exception &)
    {
        return (FT_ERR_INTERNAL);
    }
    return (FT_ERR_SUCCESSS);
}

int32_t rng_secure_bytes_with_fallback(unsigned char *buffer, ft_size_t length, int32_t *fallback_used)
{
    int32_t result;
    int32_t fallback_result;

    if (buffer == ft_nullptr && length > 0)
    {
        if (fallback_used != ft_nullptr)
            *fallback_used = 0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (fallback_used != ft_nullptr)
        *fallback_used = 0;
    result = rng_secure_bytes(buffer, length);
    if (result == FT_ERR_SUCCESSS)
        return (FT_ERR_SUCCESSS);
    if (length == 0)
        return (FT_ERR_SUCCESSS);
    fallback_result = rng_fill_from_random_device(buffer, length);
    if (fallback_result != FT_ERR_SUCCESSS)
        return (fallback_result);
    if (fallback_used != ft_nullptr)
        *fallback_used = 1;
    return (FT_ERR_SUCCESSS);
}

int32_t rng_secure_uint64(uint64_t *value, int32_t *fallback_used)
{
    int32_t result;

    if (value == ft_nullptr)
    {
        if (fallback_used != ft_nullptr)
            *fallback_used = 0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    unsigned char buffer[sizeof(uint64_t)];
    result = rng_secure_bytes_with_fallback(buffer, sizeof(uint64_t), fallback_used);
    if (result != FT_ERR_SUCCESSS)
        return (result);
    ft_size_t index = 0;
    uint64_t assembled_value = 0;
    while (index < sizeof(uint64_t))
    {
        uint64_t shifted_value = static_cast<uint64_t>(buffer[index]);
        shifted_value <<= (index * 8);
        assembled_value |= shifted_value;
        index++;
    }
    *value = assembled_value;
    return (FT_ERR_SUCCESSS);
}

int32_t rng_secure_uint32(uint32_t *value, int32_t *fallback_used)
{
    int32_t result;

    if (value == ft_nullptr)
    {
        if (fallback_used != ft_nullptr)
            *fallback_used = 0;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    uint64_t wide_value = 0;
    result = rng_secure_uint64(&wide_value, fallback_used);
    if (result != FT_ERR_SUCCESSS)
        return (result);
    *value = static_cast<uint32_t>(wide_value & 0xFFFFFFFFu);
    return (FT_ERR_SUCCESSS);
}
