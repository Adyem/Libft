#pragma once

#include <chrono>
#include <cstdio>

inline volatile void* g_efficiency_sink;

inline void prevent_optimization(void* p)
{
#if defined(__GNUG__)
    asm volatile("" : : "g"(p) : "memory");
#else
    g_efficiency_sink = p;
#endif
}

using clock_type = std::chrono::high_resolution_clock;

inline long long elapsed_us(clock_type::time_point start, clock_type::time_point end)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

inline void print_comparison(const char *name, long long std_time, long long ft_time)
{
    double percent = ft_time ? static_cast<double>(std_time) / ft_time * 100.0 : 0.0;
    printf("%s std: %lld us (100%%)\n", name, std_time);
    printf("%s ft : %lld us (%.2f%%)\n", name, ft_time, percent);
}

