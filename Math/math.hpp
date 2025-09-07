#ifndef MATH_HPP
# define MATH_HPP

int         ft_abs(int number);
long        ft_abs(long number);
long long   ft_abs(long long number);
double      ft_fabs(double number);
int         ft_signbit(double number);
int         ft_isnan(double number);
double      ft_nan(void);
void        ft_swap(int *first_number, int *second_number);
int         ft_clamp(int value, int minimum, int maximum);
double      ft_pow(double base_value, int exponent);
double      ft_sqrt(double number);
double      ft_exp(double value);

#endif
