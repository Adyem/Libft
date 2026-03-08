#ifndef MATH_FFT_HPP
#define MATH_FFT_HPP

#include "../Template/move.hpp"
#include "../Template/vector.hpp"
#include <cstdint>

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

#endif
