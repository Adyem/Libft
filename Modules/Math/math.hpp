#ifndef MATH_MATH_HPP
# define MATH_MATH_HPP

#include "../CPP_class/class_big_number.hpp"
#include "../Template/vector.hpp"
#include "ft_dual_number.hpp"
#include "math_fft.hpp"
#include "math_interval.hpp"
#include "ft_cubic_spline.hpp"
#include <cstdint>

int32_t     math_abs(int32_t number);
int64_t     math_abs(int64_t number);
double      math_fabs(double number);
double      math_fmod(double value, double modulus);
int32_t     math_signbit(double number);
int32_t     math_isnan(double number);
int32_t     math_isinf(double number);
double      math_nan(void);
double      math_infinity(void);
double      math_negative_infinity(void);
double      math_indeterminate(void);
void        math_swap(int32_t *first_number, int32_t *second_number);
int32_t     math_clamp(int32_t value, int32_t minimum, int32_t maximum);
double      math_clamp(double value, double minimum, double maximum);
int32_t     math_gcd(int32_t first_number, int32_t second_number);
int64_t     math_gcd(int64_t first_number, int64_t second_number);
ft_big_number    *math_big_gcd(const ft_big_number &first_number, const ft_big_number &second_number);
int32_t     math_lcm(int32_t first_number, int32_t second_number);
int64_t     math_lcm(int64_t first_number, int64_t second_number);
ft_big_number    *math_big_lcm(const ft_big_number &first_number, const ft_big_number &second_number);
int32_t     math_max(int32_t first_number, int32_t second_number);
int64_t     math_max(int64_t first_number, int64_t second_number);
double      math_max(double first_number, double second_number);
int32_t     math_min(int32_t first_number, int32_t second_number);
int64_t     math_min(int64_t first_number, int64_t second_number);
double      math_min(double first_number, double second_number);
int32_t     math_factorial(int32_t number);
int64_t     math_factorial(int64_t number);
int32_t     math_absdiff(int32_t first_number, int32_t second_number);
int64_t     math_absdiff(int64_t first_number, int64_t second_number);
double      math_absdiff(double first_number, double second_number);
int32_t     math_average(int32_t first_number, int32_t second_number);
int64_t     math_average(int64_t first_number, int64_t second_number);
double      math_average(double first_number, double second_number);
double      math_pow(double base_value, int32_t exponent);
double      math_sqrt(double number);
double      math_exp(double value);
double      math_log(double value);
double      math_acos(double dot);
double      math_cos(double value);
double      ft_sin(double value);
double      ft_tan(double value);
double      math_deg2rad(double degrees);
int32_t     math_validate_int(const char *input);
double      math_rad2deg(double radians);
double      ft_mean(const double *values, int32_t array_size);
double      ft_median(const double *values, int32_t array_size);
double      ft_mode(const double *values, int32_t array_size);
double      ft_variance(const double *values, int32_t array_size);
double      ft_stddev(const double *values, int32_t array_size);
double      ft_geometric_mean(const double *values, int32_t array_size);
double      ft_harmonic_mean(const double *values, int32_t array_size);
int32_t     math_fft(const ft_vector<double> &input_real,
                const ft_vector<double> &input_imag,
                ft_vector<double> &output_real,
                ft_vector<double> &output_imag) noexcept;
int32_t     math_ifft(const ft_vector<double> &input_real,
                const ft_vector<double> &input_imag,
                ft_vector<double> &output_real,
                ft_vector<double> &output_imag) noexcept;
int32_t     math_convolution(const ft_vector<double> &first,
                const ft_vector<double> &second,
                ft_vector<double> &result) noexcept;

#ifdef __APPLE__
inline int64_t math_abs(long number)
{
    return (math_abs(static_cast<int64_t>(number)));
}

inline int64_t math_gcd(long first_number, long second_number)
{
    return (math_gcd(static_cast<int64_t>(first_number),
        static_cast<int64_t>(second_number)));
}

inline int64_t math_lcm(long first_number, long second_number)
{
    return (math_lcm(static_cast<int64_t>(first_number),
        static_cast<int64_t>(second_number)));
}

inline int64_t math_max(long first_number, long second_number)
{
    return (math_max(static_cast<int64_t>(first_number),
        static_cast<int64_t>(second_number)));
}

inline int64_t math_min(long first_number, long second_number)
{
    return (math_min(static_cast<int64_t>(first_number),
        static_cast<int64_t>(second_number)));
}

inline int64_t math_factorial(long number)
{
    return (math_factorial(static_cast<int64_t>(number)));
}

inline int64_t math_absdiff(long first_number, long second_number)
{
    return (math_absdiff(static_cast<int64_t>(first_number),
        static_cast<int64_t>(second_number)));
}

inline int64_t math_average(long first_number, long second_number)
{
    return (math_average(static_cast<int64_t>(first_number),
        static_cast<int64_t>(second_number)));
}
#endif

#include "linear_algebra.hpp"

#endif
