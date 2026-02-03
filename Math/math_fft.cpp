#include "math_fft.hpp"

#include <cmath>
#include "../Errno/errno.hpp"

static const double FFT_PI = 3.141592653589793238462643383279502884;

static void math_fft_push_error(int error_code)
{
    ft_global_error_stack_push(error_code);
    return ;
}

static bool math_fft_is_power_of_two(size_t value) noexcept
{
    size_t reduced;

    if (value == 0)
        return (false);
    reduced = value;
    while ((reduced & 1) == 0)
        reduced = reduced >> 1;
    return (reduced == 1);
}

static int math_fft_copy_inputs(const ft_vector<double> &input,
    ft_vector<double> &output) noexcept
{
    size_t length;
    size_t index;

    output.clear();
    length = input.size();
    output.reserve(length);
    if (output.get_error() != FT_ERR_SUCCESSS)
    {
        math_fft_push_error(output.get_error());
        return (-1);
    }
    index = 0;
    while (index < length)
    {
        output.push_back(input[index]);
        if (output.get_error() != FT_ERR_SUCCESSS)
        {
            math_fft_push_error(output.get_error());
            output.clear();
            return (-1);
        }
        index++;
    }
    math_fft_push_error(FT_ERR_SUCCESSS);
    return (0);
}

static void math_fft_swap(ft_vector<double> &real,
    ft_vector<double> &imag, size_t first, size_t second) noexcept
{
    double temporary;

    temporary = real[first];
    real[first] = real[second];
    real[second] = temporary;
    temporary = imag[first];
    imag[first] = imag[second];
    imag[second] = temporary;
    return ;
}

static int math_fft_bit_reverse(ft_vector<double> &real,
    ft_vector<double> &imag) noexcept
{
    size_t length;
    size_t levels;
    size_t index;

    length = real.size();
    levels = 0;
    size_t value;

    value = length;
    while (value > 1)
    {
        value = value >> 1;
        levels++;
    }
    index = 0;
    while (index < length)
    {
        size_t reversed;
        size_t bit_index;

        reversed = 0;
        bit_index = 0;
        while (bit_index < levels)
        {
            reversed = (reversed << 1) | ((index >> bit_index) & 1);
            bit_index++;
        }
        if (reversed > index)
            math_fft_swap(real, imag, index, reversed);
        index++;
    }
    math_fft_push_error(FT_ERR_SUCCESSS);
    return (0);
}

static int math_fft_iterative(ft_vector<double> &real,
    ft_vector<double> &imag, bool inverse) noexcept
{
    size_t length;
    size_t half_size;

    length = real.size();
    if (length == 0)
    {
        math_fft_push_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    half_size = 1;
    while (half_size < length)
    {
        size_t step;
        double angle;
        double phase_step_real;
        double phase_step_imag;
        size_t block_start;

        step = half_size << 1;
        angle = (inverse ? 2.0 : -2.0) * FFT_PI / static_cast<double>(step);
        phase_step_real = std::cos(angle);
        phase_step_imag = std::sin(angle);
        block_start = 0;
        while (block_start < length)
        {
            double current_real;
            double current_imag;
            size_t element_index;

            current_real = 1.0;
            current_imag = 0.0;
            element_index = 0;
            while (element_index < half_size)
            {
                size_t first_index;
                size_t second_index;
                double temp_real;
                double temp_imag;
                double updated_real;
                double updated_imag;

                first_index = block_start + element_index;
                second_index = first_index + half_size;
                temp_real = current_real * real[second_index]
                    - current_imag * imag[second_index];
                temp_imag = current_real * imag[second_index]
                    + current_imag * real[second_index];
                real[second_index] = real[first_index] - temp_real;
                imag[second_index] = imag[first_index] - temp_imag;
                real[first_index] = real[first_index] + temp_real;
                imag[first_index] = imag[first_index] + temp_imag;
                updated_real = current_real * phase_step_real
                    - current_imag * phase_step_imag;
                updated_imag = current_real * phase_step_imag
                    + current_imag * phase_step_real;
                current_real = updated_real;
                current_imag = updated_imag;
                element_index++;
            }
            block_start += step;
        }
        half_size = step;
    }
    if (inverse)
    {
        double scale;
        size_t index;

        scale = 1.0 / static_cast<double>(length);
        index = 0;
        while (index < length)
        {
            real[index] = real[index] * scale;
            imag[index] = imag[index] * scale;
            index++;
        }
    }
    math_fft_push_error(FT_ERR_SUCCESSS);
    return (0);
}

static int math_fft_dispatch(const ft_vector<double> &input_real,
    const ft_vector<double> &input_imag,
    ft_vector<double> &output_real,
    ft_vector<double> &output_imag,
    bool inverse) noexcept
{
    size_t length;

    if (input_real.size() != input_imag.size())
    {
        math_fft_push_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    length = input_real.size();
    if (!math_fft_is_power_of_two(length))
    {
        math_fft_push_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (math_fft_copy_inputs(input_real, output_real) != 0)
        return (-1);
    if (math_fft_copy_inputs(input_imag, output_imag) != 0)
    {
        output_real.clear();
        return (-1);
    }
    if (math_fft_bit_reverse(output_real, output_imag) != 0)
    {
        output_real.clear();
        output_imag.clear();
        return (-1);
    }
    if (math_fft_iterative(output_real, output_imag, inverse) != 0)
    {
        output_real.clear();
        output_imag.clear();
        return (-1);
    }
    math_fft_push_error(FT_ERR_SUCCESSS);
    return (0);
}

int math_fft(const ft_vector<double> &input_real,
    const ft_vector<double> &input_imag,
    ft_vector<double> &output_real,
    ft_vector<double> &output_imag) noexcept
{
    return (math_fft_dispatch(input_real, input_imag, output_real,
            output_imag, false));
}

int math_ifft(const ft_vector<double> &input_real,
    const ft_vector<double> &input_imag,
    ft_vector<double> &output_real,
    ft_vector<double> &output_imag) noexcept
{
    return (math_fft_dispatch(input_real, input_imag, output_real,
            output_imag, true));
}

static size_t math_fft_next_power_of_two(size_t value) noexcept
{
    size_t power;

    if (value == 0)
        return (1);
    power = 1;
    while (power < value)
        power = power << 1;
    return (power);
}

static int math_fft_prepare_padded(const ft_vector<double> &input,
    size_t target_length, ft_vector<double> &real,
    ft_vector<double> &imag) noexcept
{
    size_t index;
    size_t length;

    real.clear();
    imag.clear();
    real.reserve(target_length);
    imag.reserve(target_length);
    if (real.get_error() != FT_ERR_SUCCESSS || imag.get_error() != FT_ERR_SUCCESSS)
    {
        if (real.get_error() != FT_ERR_SUCCESSS)
            math_fft_push_error(real.get_error());
        else
            math_fft_push_error(imag.get_error());
        real.clear();
        imag.clear();
        return (-1);
    }
    length = input.size();
    index = 0;
    while (index < target_length)
    {
        double value;

        if (index < length)
            value = input[index];
        else
            value = 0.0;
        real.push_back(value);
        if (real.get_error() != FT_ERR_SUCCESSS)
        {
            math_fft_push_error(real.get_error());
            real.clear();
            imag.clear();
            return (-1);
        }
        imag.push_back(0.0);
        if (imag.get_error() != FT_ERR_SUCCESSS)
        {
            math_fft_push_error(imag.get_error());
            real.clear();
            imag.clear();
            return (-1);
        }
        index++;
    }
    math_fft_push_error(FT_ERR_SUCCESSS);
    return (0);
}

static int math_fft_pointwise_multiply(ft_vector<double> &first_real,
    ft_vector<double> &first_imag,
    const ft_vector<double> &second_real,
    const ft_vector<double> &second_imag) noexcept
{
    size_t length;
    size_t index;

    length = first_real.size();
    if (second_real.size() != length || second_imag.size() != length)
    {
        math_fft_push_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    index = 0;
    while (index < length)
    {
        double result_real;
        double result_imag;

        result_real = first_real[index] * second_real[index]
            - first_imag[index] * second_imag[index];
        result_imag = first_real[index] * second_imag[index]
            + first_imag[index] * second_real[index];
        first_real[index] = result_real;
        first_imag[index] = result_imag;
        index++;
    }
    math_fft_push_error(FT_ERR_SUCCESSS);
    return (0);
}

int math_convolution(const ft_vector<double> &first,
    const ft_vector<double> &second,
    ft_vector<double> &result) noexcept
{
    size_t result_length;
    size_t fft_length;
    ft_vector<double> first_real;
    ft_vector<double> first_imag;
    ft_vector<double> second_real;
    ft_vector<double> second_imag;
    ft_vector<double> first_fft_real;
    ft_vector<double> first_fft_imag;
    ft_vector<double> second_fft_real;
    ft_vector<double> second_fft_imag;
    size_t index;

    if (first.size() == 0 || second.size() == 0)
    {
        math_fft_push_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    result_length = first.size() + second.size() - 1;
    fft_length = math_fft_next_power_of_two(result_length);
    if (math_fft_prepare_padded(first, fft_length, first_real, first_imag) != 0)
        return (-1);
    if (math_fft_prepare_padded(second, fft_length, second_real, second_imag) != 0)
        return (-1);
    if (math_fft(first_real, first_imag, first_fft_real, first_fft_imag) != 0)
        return (-1);
    if (math_fft(second_real, second_imag, second_fft_real, second_fft_imag) != 0)
        return (-1);
    if (math_fft_pointwise_multiply(first_fft_real, first_fft_imag,
            second_fft_real, second_fft_imag) != 0)
        return (-1);
    if (math_ifft(first_fft_real, first_fft_imag,
            second_fft_real, second_fft_imag) != 0)
        return (-1);
    result.clear();
    result.reserve(result_length);
    if (result.get_error() != FT_ERR_SUCCESSS)
    {
        math_fft_push_error(result.get_error());
        result.clear();
        return (-1);
    }
    index = 0;
    while (index < result_length)
    {
        double value;

        value = second_fft_real[index];
        if (std::fabs(value) < 1e-12)
            value = 0.0;
        result.push_back(value);
        if (result.get_error() != FT_ERR_SUCCESSS)
        {
            math_fft_push_error(result.get_error());
            result.clear();
            return (-1);
        }
        index++;
    }
    math_fft_push_error(FT_ERR_SUCCESSS);
    return (0);
}
