#include "class_big_number.hpp"
#include "class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include <limits>

struct ft_big_number_digit_entry
{
    char    symbol;
    int     value;
};

static const ft_big_number_digit_entry g_big_number_digit_to_value_table[] =
{
    {'0', 0},
    {'1', 1},
    {'2', 2},
    {'3', 3},
    {'4', 4},
    {'5', 5},
    {'6', 6},
    {'7', 7},
    {'8', 8},
    {'9', 9},
    {'A', 10},
    {'B', 11},
    {'C', 12},
    {'D', 13},
    {'E', 14},
    {'F', 15},
    {'a', 10},
    {'b', 11},
    {'c', 12},
    {'d', 13},
    {'e', 14},
    {'f', 15},
    {'\0', -1}
};

static const char g_big_number_value_to_digit_table[] = "0123456789ABCDEF";

static int ft_big_number_digit_value(char digit) noexcept
{
    ft_size_t index = 0;

    while (g_big_number_digit_to_value_table[index].symbol != '\0')
    {
        if (g_big_number_digit_to_value_table[index].symbol == digit)
            return (g_big_number_digit_to_value_table[index].value);
        index++;
    }
    return (-1);
}

static char ft_big_number_digit_symbol(int value) noexcept
{
    if (value < 0 || value > 15)
        return ('\0');
    return (g_big_number_value_to_digit_table[value]);
}

ft_big_number::ft_big_number() noexcept
    : _digits(ft_nullptr)
    , _size(0)
    , _capacity(0)
    , _is_negative(false)
    , _error_code(0)
{
    return ;
}

ft_big_number::ft_big_number(const ft_big_number& other) noexcept
    : _digits(ft_nullptr)
    , _size(other._size)
    , _capacity(other._capacity)
    , _is_negative(other._is_negative)
    , _error_code(other._error_code)
{
    if (other._digits)
    {
        this->_digits = static_cast<char*>(cma_calloc(this->_capacity, sizeof(char)));
        if (!this->_digits)
        {
            this->_size = 0;
            this->_capacity = 0;
            this->set_error(BIG_NUMBER_ALLOC_FAIL);
            return ;
        }
        ft_memcpy(this->_digits, other._digits, other._size + 1);
    }
    return ;
}

ft_big_number::ft_big_number(ft_big_number&& other) noexcept
    : _digits(other._digits)
    , _size(other._size)
    , _capacity(other._capacity)
    , _is_negative(other._is_negative)
    , _error_code(other._error_code)
{
    other._digits = ft_nullptr;
    other._size = 0;
    other._capacity = 0;
    other._is_negative = false;
    other._error_code = 0;
    return ;
}

ft_big_number::~ft_big_number()
{
    cma_free(this->_digits);
    return ;
}

ft_big_number& ft_big_number::operator=(const ft_big_number& other) noexcept
{
    if (this == &other)
        return (*this);
    char* new_digits = ft_nullptr;
    if (other._digits)
    {
        new_digits = static_cast<char*>(cma_calloc(other._capacity, sizeof(char)));
        if (!new_digits)
        {
            this->set_error(BIG_NUMBER_ALLOC_FAIL);
            return (*this);
        }
        ft_memcpy(new_digits, other._digits, other._size + 1);
    }
    cma_free(this->_digits);
    this->_digits = new_digits;
    this->_size = other._size;
    this->_capacity = other._capacity;
    this->_is_negative = other._is_negative;
    this->_error_code = other._error_code;
    return (*this);
}

ft_big_number& ft_big_number::operator=(ft_big_number&& other) noexcept
{
    if (this != &other)
    {
        cma_free(this->_digits);
        this->_digits = other._digits;
        this->_size = other._size;
        this->_capacity = other._capacity;
        this->_is_negative = other._is_negative;
        this->_error_code = other._error_code;
        other._digits = ft_nullptr;
        other._size = 0;
        other._capacity = 0;
        other._is_negative = false;
        other._error_code = 0;
    }
    return (*this);
}

void ft_big_number::set_error(int error_code) noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

bool ft_big_number::is_zero_value() const noexcept
{
    if (this->_size == 0)
        return (true);
    if (!this->_digits)
        return (true);
    ft_size_t digit_index = 0;
    while (digit_index < this->_size)
    {
        if (this->_digits[digit_index] != '0')
            return (false);
        digit_index++;
    }
    return (true);
}

int ft_big_number::compare_magnitude(const ft_big_number& other) const noexcept
{
    if (this->is_zero_value() && other.is_zero_value())
        return (0);
    if (this->is_zero_value())
        return (-1);
    if (other.is_zero_value())
        return (1);
    if (this->_size > other._size)
        return (1);
    if (this->_size < other._size)
        return (-1);
    ft_size_t digit_index = 0;
    while (digit_index < this->_size)
    {
        char left_digit = this->_digits[digit_index];
        char right_digit = other._digits[digit_index];
        if (left_digit > right_digit)
            return (1);
        if (left_digit < right_digit)
            return (-1);
        digit_index++;
    }
    return (0);
}

ft_big_number ft_big_number::add_magnitude(const ft_big_number& other) const noexcept
{
    ft_big_number result;
    if (this->is_zero_value())
    {
        result = other;
        result._is_negative = false;
        return (result);
    }
    if (other.is_zero_value())
    {
        result = *this;
        result._is_negative = false;
        return (result);
    }
    ft_size_t left_length = this->_size;
    ft_size_t right_length = other._size;
    ft_size_t max_length = left_length;
    if (max_length < right_length)
        max_length = right_length;
    max_length++;
    result.reserve(max_length + 1);
    if (result._error_code != 0)
        return (result);
    result._size = max_length;
    result._digits[max_length] = '\0';
    ft_size_t left_index = left_length;
    ft_size_t right_index = right_length;
    ft_size_t write_index = max_length;
    int carry_value = 0;
    while (left_index > 0 || right_index > 0 || carry_value > 0)
    {
        int digit_sum = carry_value;
        if (left_index > 0)
        {
            left_index--;
            digit_sum += this->_digits[left_index] - '0';
        }
        if (right_index > 0)
        {
            right_index--;
            digit_sum += other._digits[right_index] - '0';
        }
        write_index--;
        result._digits[write_index] = static_cast<char>('0' + (digit_sum % 10));
        carry_value = digit_sum / 10;
    }
    ft_size_t used_digits = max_length - write_index;
    if (used_digits == 0)
    {
        result._digits[0] = '0';
        result._digits[1] = '\0';
        result._size = 1;
    }
    else
    {
        ft_memmove(result._digits, result._digits + write_index, used_digits);
        result._digits[used_digits] = '\0';
        result._size = used_digits;
    }
    result.shrink_capacity();
    result._is_negative = false;
    return (result);
}

ft_big_number ft_big_number::subtract_magnitude(const ft_big_number& other) const noexcept
{
    ft_big_number result;
    int comparison = this->compare_magnitude(other);
    if (comparison < 0)
    {
        result.set_error(BIG_NUMBER_NEGATIVE_RESULT);
        return (result);
    }
    if (comparison == 0)
        return (result);
    if (other.is_zero_value())
    {
        result = *this;
        result._is_negative = false;
        return (result);
    }
    result.reserve(this->_size + 1);
    if (result._error_code != 0)
        return (result);
    result._size = this->_size;
    result._digits[this->_size] = '\0';
    ft_size_t left_index = this->_size;
    ft_size_t right_index = other._size;
    int borrow_value = 0;
    while (left_index > 0)
    {
        left_index--;
        int left_digit_value = this->_digits[left_index] - '0';
        if (borrow_value > 0)
            left_digit_value--;
        if (right_index > 0)
        {
            right_index--;
            int right_digit_value = other._digits[right_index] - '0';
            left_digit_value -= right_digit_value;
        }
        if (left_digit_value < 0)
        {
            left_digit_value += 10;
            borrow_value = 1;
        }
        else
            borrow_value = 0;
        result._digits[left_index] = static_cast<char>('0' + left_digit_value);
    }
    result.trim_leading_zeros();
    result._is_negative = false;
    return (result);
}

void ft_big_number::reserve(ft_size_t new_capacity) noexcept
{
    if (new_capacity <= this->_capacity)
        return ;
    if (new_capacity < this->_size + 1)
        new_capacity = this->_size + 1;
    if (new_capacity < 16)
        new_capacity = 16;
    char* new_digits;
    if (this->_digits)
        new_digits = static_cast<char*>(cma_realloc(this->_digits, new_capacity));
    else
        new_digits = static_cast<char*>(cma_calloc(new_capacity, sizeof(char)));
    if (!new_digits)
    {
        this->set_error(BIG_NUMBER_ALLOC_FAIL);
        return ;
    }
    this->_digits = new_digits;
    this->_capacity = new_capacity;
    return ;
}

void ft_big_number::shrink_capacity() noexcept
{
    if (!this->_digits)
        return ;
    if (this->_size == 0)
    {
        cma_free(this->_digits);
        this->_digits = ft_nullptr;
        this->_capacity = 0;
        return ;
    }
    ft_size_t desired_capacity = this->_size + 1;
    if (desired_capacity < 16)
        desired_capacity = 16;
    ft_size_t shrink_threshold = desired_capacity;
    if (desired_capacity <= std::numeric_limits<ft_size_t>::max() / 2)
        shrink_threshold = desired_capacity * 2;
    else
        shrink_threshold = std::numeric_limits<ft_size_t>::max();
    if (this->_capacity <= shrink_threshold)
        return ;
    char* new_digits = static_cast<char*>(cma_realloc(this->_digits, desired_capacity));
    if (!new_digits)
    {
        this->set_error(BIG_NUMBER_ALLOC_FAIL);
        return ;
    }
    this->_digits = new_digits;
    this->_capacity = desired_capacity;
    return ;
}

void ft_big_number::assign(const char* number) noexcept
{
    if (!number)
    {
        this->clear();
        return ;
    }
    this->_error_code = ER_SUCCESS;
    ft_errno = ER_SUCCESS;
    bool parsed_negative = false;
    ft_size_t start_index = 0;
    if (number[start_index] == '-')
    {
        parsed_negative = true;
        start_index++;
    }
    ft_size_t length = 0;
    ft_size_t index = start_index;
    while (number[index] != '\0')
    {
        if (number[index] < '0' || number[index] > '9')
        {
            this->set_error(BIG_NUMBER_INVALID_DIGIT);
            return ;
        }
        length++;
        index++;
    }
    if (length == 0)
    {
        if (parsed_negative)
        {
            this->set_error(BIG_NUMBER_INVALID_DIGIT);
            return ;
        }
        this->clear();
        return ;
    }
    if (length + 1 > this->_capacity || !this->_digits)
    {
        this->reserve(length + 1);
        if (this->_error_code)
            return ;
    }
    ft_memcpy(this->_digits, number + start_index, length);
    this->_digits[length] = '\0';
    this->_size = length;
    this->_is_negative = parsed_negative;
    this->trim_leading_zeros();
    if (this->is_zero_value())
        this->_is_negative = false;
    if (!this->_error_code)
        this->shrink_capacity();
    return ;
}

void ft_big_number::assign_base(const char* digits, int base) noexcept
{
    if (!digits)
    {
        this->clear();
        return ;
    }
    this->_error_code = ER_SUCCESS;
    ft_errno = ER_SUCCESS;
    int local_error = 0;
    bool parsed_negative = false;
    ft_size_t index = 0;

    if (base < 2 || base > 16)
        local_error = BIG_NUMBER_INVALID_DIGIT;
    else
    {
        if (digits[index] == '-')
        {
            parsed_negative = true;
            index++;
        }
        if (digits[index] == '\0')
            local_error = BIG_NUMBER_INVALID_DIGIT;
        else
        {
            ft_big_number base_number;

            base_number.append_unsigned(static_cast<unsigned long>(base));
            if (base_number.get_error() != 0)
                local_error = base_number.get_error();
            else
            {
                ft_big_number result;

                while (digits[index] != '\0')
                {
                    int digit_value = ft_big_number_digit_value(digits[index]);

                    if (digit_value < 0 || digit_value >= base)
                    {
                        local_error = BIG_NUMBER_INVALID_DIGIT;
                        break;
                    }
                    ft_big_number product = result * base_number;
                    if (product.get_error() != 0)
                    {
                        local_error = product.get_error();
                        break;
                    }
                    result = product;
                    if (result.get_error() != 0)
                    {
                        local_error = result.get_error();
                        break;
                    }
                    ft_big_number addend;

                    addend.append_unsigned(static_cast<unsigned long>(digit_value));
                    if (addend.get_error() != 0)
                    {
                        local_error = addend.get_error();
                        break;
                    }
                    ft_big_number sum = result + addend;
                    if (sum.get_error() != 0)
                    {
                        local_error = sum.get_error();
                        break;
                    }
                    result = sum;
                    if (result.get_error() != 0)
                    {
                        local_error = result.get_error();
                        break;
                    }
                    index++;
                }
                if (local_error == 0)
                {
                    if (parsed_negative && result.is_zero_value())
                        parsed_negative = false;
                    *this = result;
                    if (this->_error_code != 0)
                        local_error = this->_error_code;
                    else
                    {
                        this->_is_negative = parsed_negative;
                        if (this->is_zero_value())
                            this->_is_negative = false;
                        this->shrink_capacity();
                    }
                }
            }
        }
    }
    if (local_error != 0)
    {
        this->_error_code = local_error;
        ft_errno = local_error;
    }
    return ;
}

void ft_big_number::append_digit(char digit) noexcept
{
    if (digit < '0' || digit > '9')
    {
        this->set_error(BIG_NUMBER_INVALID_DIGIT);
        return ;
    }
    ft_size_t required_capacity = this->_size + 2;
    if (required_capacity > this->_capacity)
    {
        ft_size_t new_capacity = this->_capacity;
        if (new_capacity == 0)
            new_capacity = 16;
        while (new_capacity < required_capacity)
        {
            if (new_capacity > std::numeric_limits<ft_size_t>::max() / 2)
            {
                new_capacity = required_capacity;
                break;
            }
            new_capacity *= 2;
        }
        this->reserve(new_capacity);
        if (this->_error_code)
            return ;
    }
    if (!this->_digits)
    {
        this->reserve(required_capacity);
        if (this->_error_code)
            return ;
    }
    if (this->_size == 1 && this->_digits[0] == '0')
        this->_size = 0;
    this->_digits[this->_size] = digit;
    this->_size++;
    this->_digits[this->_size] = '\0';
    return ;
}

void ft_big_number::append(const char* digits) noexcept
{
    if (!digits)
        return ;
    ft_size_t digit_index = 0;
    while (digits[digit_index] != '\0')
    {
        this->append_digit(digits[digit_index]);
        if (this->_error_code)
            return ;
        digit_index++;
    }
    return ;
}

void ft_big_number::append_unsigned(unsigned long value) noexcept
{
    char digit_buffer[32];
    ft_size_t buffer_index = 0;
    if (value == 0)
    {
        digit_buffer[buffer_index] = '0';
        buffer_index++;
    }
    while (value > 0)
    {
        unsigned long remainder = value % 10;
        digit_buffer[buffer_index] = static_cast<char>('0' + remainder);
        buffer_index++;
        value /= 10;
    }
    while (buffer_index > 0)
    {
        buffer_index--;
        this->append_digit(digit_buffer[buffer_index]);
        if (this->_error_code)
            return ;
    }
    return ;
}

void ft_big_number::trim_leading_zeros() noexcept
{
    if (!this->_digits || this->_size == 0)
        return ;
    ft_size_t leading_index = 0;
    while (leading_index + 1 < this->_size && this->_digits[leading_index] == '0')
        leading_index++;
    if (leading_index == 0)
        return ;
    ft_size_t new_size = this->_size - leading_index;
    ft_memmove(this->_digits, this->_digits + leading_index, new_size);
    this->_size = new_size;
    this->_digits[this->_size] = '\0';
    this->shrink_capacity();
    return ;
}

void ft_big_number::reduce_to(ft_size_t new_size) noexcept
{
    if (new_size >= this->_size)
        return ;
    this->_size = new_size;
    if (this->_digits)
    {
        this->_digits[this->_size] = '\0';
        if (this->_size == 0)
            this->_digits[0] = '\0';
    }
    this->shrink_capacity();
    return ;
}

void ft_big_number::clear() noexcept
{
    this->_size = 0;
    if (this->_digits)
        this->_digits[0] = '\0';
    this->_is_negative = false;
    this->_error_code = ER_SUCCESS;
    ft_errno = ER_SUCCESS;
    this->shrink_capacity();
    return ;
}

ft_big_number ft_big_number::operator+(const ft_big_number& other) const noexcept
{
    if (this->_error_code != 0 || other._error_code != 0)
    {
        ft_big_number error_result;
        if (this->_error_code != 0)
            error_result.set_error(this->_error_code);
        else
            error_result.set_error(other._error_code);
        return (error_result);
    }
    ft_big_number result;
    if (this->_is_negative == other._is_negative)
    {
        result = this->add_magnitude(other);
        if (result._error_code != 0)
            return (result);
        result._is_negative = this->_is_negative;
        if (result.is_zero_value())
            result._is_negative = false;
        return (result);
    }
    int comparison = this->compare_magnitude(other);
    if (comparison == 0)
        return (result);
    if (comparison > 0)
    {
        result = this->subtract_magnitude(other);
        if (result._error_code != 0)
            return (result);
        result._is_negative = this->_is_negative;
    }
    else
    {
        result = other.subtract_magnitude(*this);
        if (result._error_code != 0)
            return (result);
        result._is_negative = other._is_negative;
    }
    if (result.is_zero_value())
        result._is_negative = false;
    return (result);
}

ft_big_number ft_big_number::operator-(const ft_big_number& other) const noexcept
{
    if (this->_error_code != 0 || other._error_code != 0)
    {
        ft_big_number error_result;
        if (this->_error_code != 0)
            error_result.set_error(this->_error_code);
        else
            error_result.set_error(other._error_code);
        return (error_result);
    }
    ft_big_number neg_other(other);
    if (neg_other._error_code != 0)
    {
        ft_big_number error_result;
        error_result.set_error(neg_other._error_code);
        return (error_result);
    }
    if (neg_other.is_zero_value())
        neg_other._is_negative = false;
    else
        neg_other._is_negative = !other._is_negative;
    ft_big_number result = this->operator+(neg_other);
    return (result);
}

ft_big_number ft_big_number::operator*(const ft_big_number& other) const noexcept
{
    if (this->_error_code != 0 || other._error_code != 0)
    {
        ft_big_number error_result;
        if (this->_error_code != 0)
            error_result.set_error(this->_error_code);
        else
            error_result.set_error(other._error_code);
        return (error_result);
    }
    ft_big_number result;
    if (this->is_zero_value() || other.is_zero_value())
        return (result);
    ft_size_t left_length = this->_size;
    ft_size_t right_length = other._size;
    ft_size_t result_length = left_length + right_length;
    result.reserve(result_length + 1);
    if (result._error_code != 0)
        return (result);
    ft_size_t fill_index = 0;
    while (fill_index < result_length)
    {
        result._digits[fill_index] = '0';
        fill_index++;
    }
    result._digits[result_length] = '\0';
    result._size = result_length;
    ft_size_t left_offset = 0;
    while (left_offset < left_length)
    {
        ft_size_t left_index = left_length - 1 - left_offset;
        int left_digit_value = this->_digits[left_index] - '0';
        int carry_value = 0;
        ft_size_t right_offset = 0;
        while (right_offset < right_length)
        {
            ft_size_t right_index = right_length - 1 - right_offset;
            int right_digit_value = other._digits[right_index] - '0';
            ft_size_t result_position = result_length - 1 - (left_offset + right_offset);
            int existing_value = result._digits[result_position] - '0';
            int total_value = left_digit_value * right_digit_value + existing_value + carry_value;
            result._digits[result_position] = static_cast<char>('0' + (total_value % 10));
            carry_value = total_value / 10;
            right_offset++;
        }
        ft_size_t carry_position = result_length - 1 - (left_offset + right_length);
        while (carry_value > 0)
        {
            int existing_value = result._digits[carry_position] - '0';
            int total_value = existing_value + carry_value;
            result._digits[carry_position] = static_cast<char>('0' + (total_value % 10));
            carry_value = total_value / 10;
            if (carry_position == 0)
                break;
            carry_position--;
        }
        left_offset++;
    }
    result.trim_leading_zeros();
    if (result._error_code != 0)
        return (result);
    if (this->_is_negative != other._is_negative)
        result._is_negative = true;
    if (result.is_zero_value())
        result._is_negative = false;
    return (result);
}

ft_big_number ft_big_number::operator/(const ft_big_number& other) const noexcept
{
    if (this->_error_code != 0 || other._error_code != 0)
    {
        ft_big_number error_result;
        if (this->_error_code != 0)
            error_result.set_error(this->_error_code);
        else
            error_result.set_error(other._error_code);
        return (error_result);
    }
    ft_big_number result;
    if (other.is_zero_value())
    {
        result.set_error(BIG_NUMBER_DIVIDE_BY_ZERO);
        return (result);
    }
    if (this->is_zero_value())
        return (result);
    int magnitude_comparison = this->compare_magnitude(other);
    if (magnitude_comparison < 0)
        return (result);
    if (magnitude_comparison == 0)
    {
        result.append_digit('1');
        if (result._error_code != 0)
            return (result);
        if (this->_is_negative != other._is_negative)
            result._is_negative = true;
        if (result.is_zero_value())
            result._is_negative = false;
        return (result);
    }
    ft_big_number remainder;
    ft_size_t digit_index = 0;
    while (digit_index < this->_size)
    {
        remainder.append_digit(this->_digits[digit_index]);
        if (remainder._error_code != 0)
        {
            result.set_error(remainder._error_code);
            return (result);
        }
        remainder.trim_leading_zeros();
        if (remainder._error_code != 0)
        {
            result.set_error(remainder._error_code);
            return (result);
        }
        int subtract_count = 0;
        int comparison = remainder.compare_magnitude(other);
        while (comparison >= 0)
        {
            remainder = remainder.subtract_magnitude(other);
            if (remainder._error_code != 0)
            {
                result.set_error(remainder._error_code);
                return (result);
            }
            subtract_count++;
            comparison = remainder.compare_magnitude(other);
        }
        char quotient_digit = static_cast<char>('0' + subtract_count);
        result.append_digit(quotient_digit);
        if (result._error_code != 0)
            return (result);
        digit_index++;
    }
    result.trim_leading_zeros();
    if (result._error_code != 0)
        return (result);
    if (this->_is_negative != other._is_negative)
        result._is_negative = true;
    if (result.is_zero_value())
        result._is_negative = false;
    return (result);
}

ft_big_number ft_big_number::operator%(const ft_big_number& other) const noexcept
{
    if (this->_error_code != 0 || other._error_code != 0)
    {
        ft_big_number error_result;
        if (this->_error_code != 0)
            error_result.set_error(this->_error_code);
        else
            error_result.set_error(other._error_code);
        return (error_result);
    }
    ft_big_number result;
    if (other.is_zero_value())
    {
        result.set_error(BIG_NUMBER_DIVIDE_BY_ZERO);
        return (result);
    }
    if (this->is_zero_value())
        return (result);
    ft_big_number dividend(*this);
    if (dividend._error_code != 0)
    {
        result.set_error(dividend._error_code);
        return (result);
    }
    dividend._is_negative = false;
    dividend.trim_leading_zeros();
    ft_big_number divisor(other);
    if (divisor._error_code != 0)
    {
        result.set_error(divisor._error_code);
        return (result);
    }
    divisor._is_negative = false;
    divisor.trim_leading_zeros();
    int magnitude_comparison = dividend.compare_magnitude(divisor);
    if (magnitude_comparison < 0)
        result = dividend;
    else if (magnitude_comparison > 0)
    {
        ft_big_number current_remainder;
        ft_size_t digit_index = 0;
        while (digit_index < dividend._size)
        {
            current_remainder.append_digit(dividend._digits[digit_index]);
            if (current_remainder._error_code != 0)
            {
                result.set_error(current_remainder._error_code);
                return (result);
            }
            current_remainder.trim_leading_zeros();
            int compare_value = current_remainder.compare_magnitude(divisor);
            while (compare_value >= 0)
            {
                current_remainder = current_remainder.subtract_magnitude(divisor);
                if (current_remainder._error_code != 0)
                {
                    result.set_error(current_remainder._error_code);
                    return (result);
                }
                compare_value = current_remainder.compare_magnitude(divisor);
            }
            digit_index++;
        }
        result = current_remainder;
    }
    result.trim_leading_zeros();
    if (this->_is_negative && !result.is_zero_value())
        result._is_negative = true;
    else
        result._is_negative = false;
    if (result.is_zero_value())
        result._is_negative = false;
    return (result);
}

bool ft_big_number::operator!=(const ft_big_number& other) const noexcept
{
    if (this->operator==(other))
        return (false);
    return (true);
}

bool ft_big_number::operator<(const ft_big_number& other) const noexcept
{
    if (this->_error_code != 0 || other._error_code != 0)
        return (false);
    if (this->operator==(other))
        return (false);
    if (this->_is_negative && !other._is_negative)
        return (true);
    if (!this->_is_negative && other._is_negative)
        return (false);
    int magnitude_comparison = this->compare_magnitude(other);
    if (this->_is_negative && other._is_negative)
    {
        if (magnitude_comparison > 0)
            return (true);
        return (false);
    }
    if (magnitude_comparison < 0)
        return (true);
    return (false);
}

bool ft_big_number::operator<=(const ft_big_number& other) const noexcept
{
    if (this->operator<(other))
        return (true);
    return (this->operator==(other));
}

bool ft_big_number::operator>(const ft_big_number& other) const noexcept
{
    if (other.operator<(*this))
        return (true);
    return (false);
}

bool ft_big_number::operator>=(const ft_big_number& other) const noexcept
{
    if (this->operator>(other))
        return (true);
    return (this->operator==(other));
}

bool ft_big_number::operator==(const ft_big_number& other) const noexcept
{
    if (this->_error_code != 0 || other._error_code != 0)
        return (false);
    if (this->is_zero_value() && other.is_zero_value())
        return (true);
    if (this->_is_negative != other._is_negative)
        return (false);
    if (this->_size != other._size)
        return (false);
    if (this->_size == 0)
        return (true);
    if (!this->_digits || !other._digits)
        return (false);
    ft_size_t digit_index = 0;
    while (digit_index < this->_size)
    {
        if (this->_digits[digit_index] != other._digits[digit_index])
            return (false);
        digit_index++;
    }
    return (true);
}

const char* ft_big_number::c_str() const noexcept
{
    if (this->_digits && this->_size > 0)
        return (this->_digits);
    return ("0");
}

ft_size_t ft_big_number::size() const noexcept
{
    return (this->_size);
}

bool ft_big_number::empty() const noexcept
{
    return (this->_size == 0);
}

bool ft_big_number::is_negative() const noexcept
{
    if (this->is_zero_value())
        return (false);
    return (this->_is_negative);
}

bool ft_big_number::is_positive() const noexcept
{
    if (this->is_zero_value())
        return (false);
    return (!this->_is_negative);
}

ft_string ft_big_number::to_string_base(int base) noexcept
{
    if (base < 2 || base > 16)
    {
        this->set_error(BIG_NUMBER_INVALID_DIGIT);
        return (ft_string(BIG_NUMBER_INVALID_DIGIT));
    }
    if (this->_error_code != 0)
        return (ft_string(this->_error_code));
    if (this->is_zero_value())
    {
        ft_string zero_result;

        zero_result.append('0');
        if (zero_result.get_error() != 0)
        {
            this->set_error(BIG_NUMBER_ALLOC_FAIL);
            return (ft_string(BIG_NUMBER_ALLOC_FAIL));
        }
        return (zero_result);
    }
    ft_big_number magnitude(*this);

    magnitude._is_negative = false;
    ft_big_number base_value;

    base_value.append_unsigned(static_cast<unsigned long>(base));
    if (base_value.get_error() != 0)
    {
        this->set_error(base_value.get_error());
        return (ft_string(base_value.get_error()));
    }
    ft_string digits;

    while (!magnitude.is_zero_value())
    {
        ft_big_number quotient = magnitude / base_value;

        if (quotient.get_error() != 0)
        {
            this->set_error(quotient.get_error());
            return (ft_string(quotient.get_error()));
        }
        ft_big_number product = quotient * base_value;

        if (product.get_error() != 0)
        {
            this->set_error(product.get_error());
            return (ft_string(product.get_error()));
        }
        ft_big_number remainder_number = magnitude - product;

        if (remainder_number.get_error() != 0)
        {
            this->set_error(remainder_number.get_error());
            return (ft_string(remainder_number.get_error()));
        }
        const char* remainder_digits = remainder_number.c_str();
        ft_size_t remainder_index = 0;
        int remainder_value = 0;

        while (remainder_digits[remainder_index] != '\0')
        {
            remainder_value = remainder_value * 10 + (remainder_digits[remainder_index] - '0');
            remainder_index++;
        }
        char digit_symbol = ft_big_number_digit_symbol(remainder_value);

        if (digit_symbol == '\0')
        {
            this->set_error(BIG_NUMBER_INVALID_DIGIT);
            return (ft_string(BIG_NUMBER_INVALID_DIGIT));
        }
        digits.append(digit_symbol);
        if (digits.get_error() != 0)
        {
            this->set_error(BIG_NUMBER_ALLOC_FAIL);
            return (ft_string(BIG_NUMBER_ALLOC_FAIL));
        }
        magnitude = quotient;
        if (magnitude.get_error() != 0)
        {
            this->set_error(magnitude.get_error());
            return (ft_string(magnitude.get_error()));
        }
        magnitude._is_negative = false;
    }
    ft_string result;

    if (this->_is_negative)
    {
        result.append('-');
        if (result.get_error() != 0)
        {
            this->set_error(BIG_NUMBER_ALLOC_FAIL);
            return (ft_string(BIG_NUMBER_ALLOC_FAIL));
        }
    }
    const char* digits_buffer = digits.c_str();
    ft_size_t digits_length = digits.size();

    while (digits_length > 0)
    {
        digits_length--;
        result.append(digits_buffer[digits_length]);
        if (result.get_error() != 0)
        {
            this->set_error(BIG_NUMBER_ALLOC_FAIL);
            return (ft_string(BIG_NUMBER_ALLOC_FAIL));
        }
    }
    return (result);
}

ft_big_number ft_big_number::mod_pow(const ft_big_number& exponent, const ft_big_number& modulus) const noexcept
{
    if (this->_error_code != 0 || exponent._error_code != 0 || modulus._error_code != 0)
    {
        ft_big_number error_result;
        if (this->_error_code != 0)
            error_result.set_error(this->_error_code);
        else if (exponent._error_code != 0)
            error_result.set_error(exponent._error_code);
        else
            error_result.set_error(modulus._error_code);
        return (error_result);
    }
    if (modulus.is_zero_value())
    {
        ft_big_number error_result;
        error_result.set_error(BIG_NUMBER_DIVIDE_BY_ZERO);
        return (error_result);
    }
    if (modulus.is_negative())
    {
        ft_big_number error_result;
        error_result.set_error(BIG_NUMBER_INVALID_DIGIT);
        return (error_result);
    }
    if (exponent.is_negative())
    {
        ft_big_number error_result;
        error_result.set_error(BIG_NUMBER_INVALID_DIGIT);
        return (error_result);
    }
    ft_big_number modulus_value(modulus);
    if (modulus_value._error_code != 0)
    {
        ft_big_number error_result;
        error_result.set_error(modulus_value._error_code);
        return (error_result);
    }
    modulus_value._is_negative = false;
    modulus_value.trim_leading_zeros();
    ft_big_number base_remainder = this->operator%(modulus_value);
    if (base_remainder._error_code != 0)
    {
        ft_big_number error_result;
        error_result.set_error(base_remainder._error_code);
        return (error_result);
    }
    while (base_remainder.is_negative())
    {
        ft_big_number adjusted_base = base_remainder + modulus_value;
        if (adjusted_base._error_code != 0)
        {
            ft_big_number error_result;
            error_result.set_error(adjusted_base._error_code);
            return (error_result);
        }
        base_remainder = adjusted_base;
    }
    base_remainder.trim_leading_zeros();
    ft_big_number exponent_value(exponent);
    if (exponent_value._error_code != 0)
    {
        ft_big_number error_result;
        error_result.set_error(exponent_value._error_code);
        return (error_result);
    }
    exponent_value._is_negative = false;
    exponent_value.trim_leading_zeros();
    ft_big_number one;
    one.append_digit('1');
    if (one._error_code != 0)
    {
        ft_big_number error_result;
        error_result.set_error(one._error_code);
        return (error_result);
    }
    if (exponent_value.is_zero_value())
    {
        ft_big_number identity_result = one % modulus_value;
        if (identity_result._error_code != 0)
        {
            ft_big_number error_result;
            error_result.set_error(identity_result._error_code);
            return (error_result);
        }
        identity_result._is_negative = false;
        if (identity_result.is_zero_value())
            identity_result._is_negative = false;
        identity_result.trim_leading_zeros();
        return (identity_result);
    }
    ft_big_number accumulator(one);
    if (accumulator._error_code != 0)
    {
        ft_big_number error_result;
        error_result.set_error(accumulator._error_code);
        return (error_result);
    }
    while (!exponent_value.is_zero_value())
    {
        ft_big_number product_value = accumulator * base_remainder;
        if (product_value._error_code != 0)
        {
            ft_big_number error_result;
            error_result.set_error(product_value._error_code);
            return (error_result);
        }
        ft_big_number reduced_value = product_value % modulus_value;
        if (reduced_value._error_code != 0)
        {
            ft_big_number error_result;
            error_result.set_error(reduced_value._error_code);
            return (error_result);
        }
        accumulator = reduced_value;
        ft_big_number next_exponent = exponent_value - one;
        if (next_exponent._error_code != 0)
        {
            ft_big_number error_result;
            error_result.set_error(next_exponent._error_code);
            return (error_result);
        }
        exponent_value = next_exponent;
        exponent_value._is_negative = false;
        exponent_value.trim_leading_zeros();
    }
    accumulator._is_negative = false;
    if (accumulator.is_zero_value())
        accumulator._is_negative = false;
    accumulator.trim_leading_zeros();
    return (accumulator);
}

int ft_big_number::get_error() const noexcept
{
    return (this->_error_code);
}

const char* ft_big_number::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
