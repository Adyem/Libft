#ifndef LIBFT_LIMITS_HPP
#define LIBFT_LIMITS_HPP

#include <cstdint>
#include <cinttypes>

typedef uint64_t ft_size_t;

static constexpr int32_t   FT_CHAR_BIT  = 8;

static constexpr int32_t FT_INT32_MAX = 2147483647;
static constexpr int32_t FT_INT32_MIN = (-2147483647 - 1);
static constexpr int64_t FT_LLONG_MAX = 9223372036854775807LL;
static constexpr int64_t FT_LLONG_MIN = (-9223372036854775807LL - 1LL);
static constexpr uint64_t FT_ULLONG_MAX = 18446744073709551615ULL;

static constexpr ft_size_t FT_SYSTEM_SIZE_MAX = 18446744073709551615ULL;

#define FT_PRId64 PRId64
#define FT_PRIu64 PRIu64

#endif
