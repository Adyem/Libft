#ifndef LIBFT_LIMITS_HPP
#define LIBFT_LIMITS_HPP

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

#endif
