#ifndef MATH_FFT_HPP
#define MATH_FFT_HPP

#include "../Template/vector.hpp"

int     math_fft(const ft_vector<double> &input_real,
            const ft_vector<double> &input_imag,
            ft_vector<double> &output_real,
            ft_vector<double> &output_imag) noexcept;
int     math_ifft(const ft_vector<double> &input_real,
            const ft_vector<double> &input_imag,
            ft_vector<double> &output_real,
            ft_vector<double> &output_imag) noexcept;
int     math_convolution(const ft_vector<double> &first,
            const ft_vector<double> &second,
            ft_vector<double> &result) noexcept;

#endif
