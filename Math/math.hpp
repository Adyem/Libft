#ifndef MATH_HPP
# define MATH_HPP

int         math_abs(int number);
long        math_abs(long number);
long long   math_abs(long long number);
double      math_fabs(double number);
int         math_signbit(double number);
int         math_isnan(double number);
double      math_nan(void);
void        math_swap(int *first_number, int *second_number);
int         math_clamp(int value, int minimum, int maximum);
int         math_gcd(int first_number, int second_number);
long        math_gcd(long first_number, long second_number);
long long   math_gcd(long long first_number, long long second_number);
int         math_lcm(int first_number, int second_number);
long        math_lcm(long first_number, long second_number);
long long   math_lcm(long long first_number, long long second_number);
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
double      math_cos(double value);
double      math_deg2rad(double degrees);
double      math_rad2deg(double radians);

#endif
