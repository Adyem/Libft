#ifndef LIBFT_LIMITS_HPP
#define LIBFT_LIMITS_HPP

#include <cstddef>

static constexpr int   FT_CHAR_BIT  = 8;

static constexpr int   FT_INT_MAX   = static_cast<int>(~0U >> 1);
static constexpr int   FT_INT_MIN   = -FT_INT_MAX - 1;
static constexpr unsigned FT_UINT_MAX = ~0U;

static constexpr long  FT_LONG_MAX  = static_cast<long>(~0UL >> 1);
static constexpr long  FT_LONG_MIN  = -FT_LONG_MAX - 1L;
static constexpr unsigned long FT_ULONG_MAX = ~0UL;

static constexpr long long  FT_LLONG_MAX  = static_cast<long long>(~0ULL >> 1);
static constexpr long long  FT_LLONG_MIN  = -FT_LLONG_MAX - 1LL;
static constexpr unsigned long long FT_ULLONG_MAX = ~0ULL;

static constexpr unsigned long long ft_compute_system_size_max()
{
    unsigned long long current_value = 0ULL;
    size_t byte_index = 0;
    while (byte_index < sizeof(size_t))
    {
        current_value = (current_value << 8U) | 0xFFULL;
        byte_index++;
    }
    return (current_value);
}

static constexpr unsigned long long FT_SYSTEM_SIZE_MAX = ft_compute_system_size_max();

#endif
