#ifndef MATH_MATH_HPP
# define MATH_MATH_HPP

# include "../CPP_class/class_big_number.hpp"
# include "../Template/vector.hpp"
# include "math_autodiff.hpp"
# include "math_fft.hpp"
# include "math_interval.hpp"
# include "math_polynomial.hpp"

int         math_abs(int number);
long        math_abs(long number);
long long   math_abs(long long number);
double      math_fabs(double number);
double      math_fmod(double value, double modulus);
int         math_signbit(double number);
int         math_isnan(double number);
int         math_isinf(double number);
double      math_nan(void);
double      math_infinity(void);
double      math_negative_infinity(void);
double      math_indeterminate(void);
void        math_swap(int *first_number, int *second_number);
int         math_clamp(int value, int minimum, int maximum);
double      math_clamp(double value, double minimum, double maximum);
int         math_gcd(int first_number, int second_number);
long        math_gcd(long first_number, long second_number);
long long   math_gcd(long long first_number, long long second_number);
ft_big_number    math_big_gcd(const ft_big_number &first_number, const ft_big_number &second_number);
int         math_lcm(int first_number, int second_number);
long        math_lcm(long first_number, long second_number);
long long   math_lcm(long long first_number, long long second_number);
ft_big_number    math_big_lcm(const ft_big_number &first_number, const ft_big_number &second_number);
int         math_max(int first_number, int second_number);
long        math_max(long first_number, long second_number);
long long   math_max(long long first_number, long long second_number);
double      math_max(double first_number, double second_number);
int         math_min(int first_number, int second_number);
long        math_min(long first_number, long second_number);
long long   math_min(long long first_number, long long second_number);
double      math_min(double first_number, double second_number);
int         math_factorial(int number);
long        math_factorial(long number);
long long   math_factorial(long long number);
int         math_absdiff(int first_number, int second_number);
long        math_absdiff(long first_number, long second_number);
long long   math_absdiff(long long first_number, long long second_number);
double      math_absdiff(double first_number, double second_number);
int         math_average(int first_number, int second_number);
long        math_average(long first_number, long second_number);
long long   math_average(long long first_number, long long second_number);
double      math_average(double first_number, double second_number);
double      math_pow(double base_value, int exponent);
double      math_sqrt(double number);
double      math_exp(double value);
double      math_log(double value);
double      math_acos(double dot);
double      math_cos(double value);
double      ft_sin(double value);
double      ft_tan(double value);
double      math_deg2rad(double degrees);
int         math_validate_int(const char *input);
double      math_rad2deg(double radians);
double      ft_mean(const double *values, int array_size);
double      ft_median(const double *values, int array_size);
double      ft_mode(const double *values, int array_size);
double      ft_variance(const double *values, int array_size);
double      ft_stddev(const double *values, int array_size);
double      ft_geometric_mean(const double *values, int array_size);
double      ft_harmonic_mean(const double *values, int array_size);
int         math_fft(const ft_vector<double> &input_real,
                const ft_vector<double> &input_imag,
                ft_vector<double> &output_real,
                ft_vector<double> &output_imag) noexcept;
int         math_ifft(const ft_vector<double> &input_real,
                const ft_vector<double> &input_imag,
                ft_vector<double> &output_real,
                ft_vector<double> &output_imag) noexcept;
int         math_convolution(const ft_vector<double> &first,
                const ft_vector<double> &second,
                ft_vector<double> &result) noexcept;

# include "linear_algebra.hpp"

#endif
