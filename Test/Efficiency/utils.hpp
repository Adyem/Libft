#pragma once

#include <cstdio>

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Time/time.hpp"

inline volatile void* g_efficiency_sink;

inline void prevent_optimization(void* p)
{
#if defined(__GNUG__)
    asm volatile("" : : "g"(p) : "memory");
#else
    g_efficiency_sink = p;
#endif
}

inline void prevent_optimization(const void *p)
{
#if defined(__GNUG__)
    asm volatile("" : : "g"(p) : "memory");
#else
    g_efficiency_sink = const_cast<void *>(p);
#endif
}

inline void prevent_optimization(const volatile void *p)
{
#if defined(__GNUG__)
    asm volatile("" : : "g"(p) : "memory");
#else
    g_efficiency_sink = const_cast<void *>(
        const_cast<const void *>(p));
#endif
}

struct ft_efficiency_clock_type
{
    typedef t_high_resolution_time_point time_point;

    static time_point now()
    {
        time_point time_point_value;

        time_point_value.nanoseconds = 0;
        (void)time_high_resolution_now(&time_point_value);
        return (time_point_value);
    }
};

using clock_type = ft_efficiency_clock_type;

inline int64_t elapsed_us(clock_type::time_point start, clock_type::time_point end)
{
    return (time_high_resolution_diff_ns(start, end) / 1000);
}

inline void print_comparison(const char *name, int64_t std_time, int64_t ft_time)
{
    double percent;
    if (ft_time)
        percent = static_cast<double>(std_time) / ft_time * 100.0;
    else
        percent = 0.0;
    printf("%s std: " FT_INT64_DECIMAL_FORMAT " us (100%%)\n", name, std_time);
    printf("%s ft : " FT_INT64_DECIMAL_FORMAT " us (%.2f%%)\n", name, ft_time, percent);
}
