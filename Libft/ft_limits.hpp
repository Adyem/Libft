#ifndef FT_LIMITS_HPP
#define FT_LIMITS_HPP

#define FT_CHAR_BIT 8

#define FT_INT_MAX ((int)(~0U >> 1))
#define FT_INT_MIN (-FT_INT_MAX - 1)
#define FT_UINT_MAX (~0U)

#define FT_LONG_MAX ((long)(~0UL >> 1))
#define FT_LONG_MIN (-FT_LONG_MAX - 1L)
#define FT_ULONG_MAX (~0UL)

#endif
