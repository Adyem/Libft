#ifndef FT_LIMITS_HPP
#define FT_LIMITS_HPP

static constexpr int   FT_CHAR_BIT  = 8;

static constexpr int   FT_INT_MAX   = static_cast<int>(~0U >> 1);
static constexpr int   FT_INT_MIN   = -FT_INT_MAX - 1;
static constexpr unsigned FT_UINT_MAX = ~0U;

static constexpr long  FT_LONG_MAX  = static_cast<long>(~0UL >> 1);
static constexpr long  FT_LONG_MIN  = -FT_LONG_MAX - 1L;
static constexpr unsigned long FT_ULONG_MAX = ~0UL;

#endif
