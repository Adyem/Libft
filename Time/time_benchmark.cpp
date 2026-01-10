#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cmath>
#include <limits>

static void time_benchmark_set_error(t_time_benchmark *benchmark, int error_code)
{
    if (benchmark != ft_nullptr)
        benchmark->error_code = error_code;
    ft_global_error_stack_push(error_code);
    return ;
}

void    time_benchmark_init(t_time_benchmark *benchmark)
{
    time_benchmark_reset(benchmark);
    return ;
}

void    time_benchmark_reset(t_time_benchmark *benchmark)
{
    if (benchmark == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    benchmark->sample_count = 0;
    benchmark->rolling_mean_ms = 0.0;
    benchmark->rolling_m2_ms = 0.0;
    benchmark->minimum_ms = 0.0;
    benchmark->maximum_ms = 0.0;
    time_benchmark_set_error(benchmark, FT_ERR_SUCCESSS);
    return ;
}

int time_benchmark_add_sample(t_time_benchmark *benchmark, double duration_ms)
{
    double  delta;
    double  delta_after_update;
    size_t  new_count;
    double  count_double;

    if (benchmark == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!(duration_ms >= 0.0))
    {
        time_benchmark_set_error(benchmark, FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!std::isfinite(duration_ms))
    {
        time_benchmark_set_error(benchmark, FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (benchmark->sample_count >= std::numeric_limits<size_t>::max())
    {
        time_benchmark_set_error(benchmark, FT_ERR_OUT_OF_RANGE);
        return (-1);
    }
    new_count = benchmark->sample_count + 1;
    count_double = static_cast<double>(new_count);
    delta = duration_ms - benchmark->rolling_mean_ms;
    benchmark->rolling_mean_ms += delta / count_double;
    delta_after_update = duration_ms - benchmark->rolling_mean_ms;
    benchmark->rolling_m2_ms += delta * delta_after_update;
    if (new_count == 1)
    {
        benchmark->minimum_ms = duration_ms;
        benchmark->maximum_ms = duration_ms;
    }
    else
    {
        if (duration_ms < benchmark->minimum_ms)
            benchmark->minimum_ms = duration_ms;
        if (duration_ms > benchmark->maximum_ms)
            benchmark->maximum_ms = duration_ms;
    }
    benchmark->sample_count = new_count;
    time_benchmark_set_error(benchmark, FT_ERR_SUCCESSS);
    return (0);
}

int time_benchmark_add_duration(t_time_benchmark *benchmark,
    t_duration_milliseconds duration)
{
    return (time_benchmark_add_sample(benchmark,
        static_cast<double>(duration.milliseconds)));
}

static double  time_benchmark_calculate_jitter(const t_time_benchmark *benchmark)
{
    double variance;

    variance = 0.0;
    if (benchmark->sample_count > 1)
    {
        variance = benchmark->rolling_m2_ms
            / static_cast<double>(benchmark->sample_count - 1);
        if (variance < 0.0)
            variance = 0.0;
    }
    return (std::sqrt(variance));
}

bool    time_benchmark_snapshot(const t_time_benchmark *benchmark,
    t_time_benchmark_snapshot *out_snapshot)
{
    double jitter_ms;

    if (benchmark == ft_nullptr || out_snapshot == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    out_snapshot->sample_count = benchmark->sample_count;
    out_snapshot->average_ms = benchmark->rolling_mean_ms;
    out_snapshot->minimum_ms = benchmark->minimum_ms;
    out_snapshot->maximum_ms = benchmark->maximum_ms;
    jitter_ms = time_benchmark_calculate_jitter(benchmark);
    out_snapshot->jitter_ms = jitter_ms;
    if (benchmark->error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(benchmark->error_code);
        return (false);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

size_t  time_benchmark_get_sample_count(const t_time_benchmark *benchmark)
{
    if (benchmark == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    ft_global_error_stack_push(benchmark->error_code);
    return (benchmark->sample_count);
}

double  time_benchmark_get_average_ms(const t_time_benchmark *benchmark)
{
    if (benchmark == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0.0);
    }
    ft_global_error_stack_push(benchmark->error_code);
    return (benchmark->rolling_mean_ms);
}

double  time_benchmark_get_jitter_ms(const t_time_benchmark *benchmark)
{
    if (benchmark == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0.0);
    }
    if (benchmark->error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(benchmark->error_code);
        return (0.0);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (time_benchmark_calculate_jitter(benchmark));
}

double  time_benchmark_get_minimum_ms(const t_time_benchmark *benchmark)
{
    if (benchmark == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0.0);
    }
    ft_global_error_stack_push(benchmark->error_code);
    return (benchmark->minimum_ms);
}

double  time_benchmark_get_maximum_ms(const t_time_benchmark *benchmark)
{
    if (benchmark == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0.0);
    }
    ft_global_error_stack_push(benchmark->error_code);
    return (benchmark->maximum_ms);
}

int time_benchmark_get_error(const t_time_benchmark *benchmark)
{
    if (benchmark == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    ft_global_error_stack_push(benchmark->error_code);
    return (benchmark->error_code);
}

const char  *time_benchmark_get_error_str(const t_time_benchmark *benchmark)
{
    if (benchmark == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_strerror(FT_ERR_INVALID_ARGUMENT));
    }
    ft_global_error_stack_push(benchmark->error_code);
    return (ft_strerror(benchmark->error_code));
}
