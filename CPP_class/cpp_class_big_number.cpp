#include "class_big_number.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"
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

thread_local ft_error_stack ft_big_number::_error_stack = {{}, 0, 1, FT_ERR_SUCCESSS, 0};
thread_local ft_operation_error_stack ft_big_number::_operation_errors = {{}, 0};

class ft_big_number::error_scope
{
    private:
        uint32_t        _index;
        uint32_t        _operation_id;
        bool            _active;

    public:
        error_scope() noexcept;
        ~error_scope() noexcept;
        void            set_error(int error_code) noexcept;
        uint32_t        get_operation_id() const noexcept;
};

ft_big_number::error_scope::error_scope() noexcept
    : _index(0)
    , _operation_id(0)
    , _active(false)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    if (ft_big_number::_error_stack.depth >= 32)
    {
        this->_operation_id = ft_big_number::_error_stack.next_op_id;
        ft_big_number::_error_stack.next_op_id++;
        return ;
    }
    this->_index = ft_big_number::_error_stack.depth;
    ft_big_number::_error_stack.depth++;
    this->_operation_id = ft_big_number::_error_stack.next_op_id;
    ft_big_number::_error_stack.next_op_id++;
    ft_big_number::_error_stack.frames[this->_index].code = FT_ERR_SUCCESSS;
    ft_big_number::_error_stack.frames[this->_index].op_id = this->_operation_id;
    this->_active = true;
    return ;
}

ft_big_number::error_scope::~error_scope() noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    if (!this->_active)
        return ;
    if (ft_big_number::_error_stack.depth == 0)
        return ;
    ft_big_number::_error_stack.last_error = ft_big_number::_error_stack.frames[this->_index].code;
    ft_big_number::_error_stack.last_op_id = ft_big_number::_error_stack.frames[this->_index].op_id;
    ft_big_number::_error_stack.depth--;
    return ;
}

void ft_big_number::error_scope::set_error(int error_code) noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    if (!this->_active)
        return ;
    ft_big_number::_error_stack.frames[this->_index].code = error_code;
    return ;
}

uint32_t ft_big_number::error_scope::get_operation_id() const noexcept
{
    return (this->_operation_id);
}

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

int ft_big_number::initialize_errno_keeper() noexcept
{
    return (FT_SYS_ERR_SUCCESS);
}

void ft_big_number::update_errno_keeper(int &stored_errno, int new_value) noexcept
{
    if (new_value != FT_SYS_ERR_SUCCESS)
        stored_errno = new_value;
    return ;
}

void ft_big_number::finalize_errno_keeper(int stored_errno) noexcept
{
    (void)stored_errno;
    return ;
}

int ft_big_number::last_error() noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    if (ft_big_number::_error_stack.depth == 0)
        return (ft_big_number::_error_stack.last_error);
    return (ft_big_number::_error_stack.frames[ft_big_number::_error_stack.depth - 1].code);
}

uint32_t ft_big_number::last_op_id() noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    if (ft_big_number::_error_stack.depth == 0)
        return (ft_big_number::_error_stack.last_op_id);
    return (ft_big_number::_error_stack.frames[ft_big_number::_error_stack.depth - 1].op_id);
}

int ft_big_number::error_for(uint32_t operation_id) noexcept
{
    uint32_t index;

    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    if (ft_big_number::_error_stack.depth == 0)
        return (FT_ERR_SUCCESSS);
    index = ft_big_number::_error_stack.depth;
    while (index > 0)
    {
        index--;
        if (ft_big_number::_error_stack.frames[index].op_id == operation_id)
            return (ft_big_number::_error_stack.frames[index].code);
    }
    if (ft_big_number::_error_stack.last_op_id == operation_id)
        return (ft_big_number::_error_stack.last_error);
    return (FT_ERR_SUCCESSS);
}

void ft_big_number::record_operation_error(int error_code) noexcept
{
    ft_size_t index;
    ft_size_t shift_index;

    if (ft_big_number::_operation_errors.count < 20)
        ft_big_number::_operation_errors.count++;
    if (ft_big_number::_operation_errors.count > 0)
        shift_index = ft_big_number::_operation_errors.count - 1;
    else
        shift_index = 0;
    index = shift_index;
    while (index > 0)
    {
        ft_big_number::_operation_errors.errors[index] =
            ft_big_number::_operation_errors.errors[index - 1];
        index--;
    }
    ft_big_number::_operation_errors.errors[0] = error_code;
    return ;
}

int ft_big_number::last_operation_error() noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    if (ft_big_number::_operation_errors.count == 0)
        return (FT_ERR_SUCCESSS);
    return (ft_big_number::_operation_errors.errors[0]);
}

int ft_big_number::operation_error_at(ft_size_t index) noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    if (index == 0 || index > ft_big_number::_operation_errors.count)
        return (FT_ERR_SUCCESSS);
    return (ft_big_number::_operation_errors.errors[index - 1]);
}

void ft_big_number::pop_operation_errors() noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    ft_big_number::_operation_errors.count = 0;
    return ;
}

int ft_big_number::pop_oldest_operation_error() noexcept
{
    int popped_error;

    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    if (ft_big_number::_operation_errors.count == 0)
        return (FT_ERR_SUCCESSS);
    popped_error = ft_big_number::_operation_errors
        .errors[ft_big_number::_operation_errors.count - 1];
    ft_big_number::_operation_errors.count--;
    return (popped_error);
}

int ft_big_number::operation_error_index() noexcept
{
    ft_size_t index;

    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    index = 0;
    while (index < ft_big_number::_operation_errors.count)
    {
        if (ft_big_number::_operation_errors.errors[index] != FT_ERR_SUCCESSS)
            return (static_cast<int>(index + 1));
        index++;
    }
    return (0);
}

void ft_big_number::unlock_guard_preserve_errno(ft_big_number_mutex_guard &guard,
        int &stored_errno) noexcept
{
    int previous_errno;
    int unlock_error;

    previous_errno = stored_errno;
    if (!guard.owns_lock())
    {
        ft_big_number::update_errno_keeper(stored_errno, previous_errno);
        return ;
    }
    guard.unlock();
    unlock_error = guard.get_error();
    if (unlock_error != FT_ERR_SUCCESSS)
        stored_errno = unlock_error;
    else
        stored_errno = previous_errno;
    return ;
}

void ft_big_number::sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

void ft_big_number::set_error_unlocked(int error_code) const noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    ft_big_number::record_operation_error(error_code);
    ft_errno = error_code;
    return ;
}

void ft_big_number::set_system_error_unlocked(int error_code) const noexcept
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    if (error_code != FT_SYS_ERR_SUCCESS)
        ft_big_number::record_operation_error(error_code);
    ft_sys_errno = error_code;
    return ;
}

void ft_big_number::set_system_error(int error_code) const noexcept
{
    this->set_system_error_unlocked(error_code);
    return ;
}

int ft_big_number::lock_self(ft_big_number_mutex_guard &guard) const noexcept
{
    ft_big_number_mutex_guard local_guard(this->_mutex);

    if (local_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_system_error(local_guard.get_error());
        guard = ft_big_number_mutex_guard();
        return (local_guard.get_error());
    }
    this->set_system_error(FT_SYS_ERR_SUCCESS);
    guard = ft_move(local_guard);
    return (FT_ERR_SUCCESSS);
}

int ft_big_number::lock_pair(const ft_big_number &first, const ft_big_number &second,
        ft_big_number_mutex_guard &first_guard,
        ft_big_number_mutex_guard &second_guard) noexcept
{
    const ft_big_number *ordered_first;
    const ft_big_number *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_big_number_mutex_guard single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            first.set_system_error(single_guard.get_error());
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_big_number_mutex_guard();
        first.set_system_error(FT_SYS_ERR_SUCCESS);
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_big_number *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_big_number_mutex_guard lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ordered_first->set_system_error(lower_guard.get_error());
            return (lower_guard.get_error());
        }
        ft_big_number_mutex_guard upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESSS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ordered_first->set_system_error(FT_SYS_ERR_SUCCESS);
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ordered_second->set_system_error(upper_guard.get_error());
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        ft_big_number::sleep_backoff();
    }
}

void ft_big_number::clear_unlocked() noexcept
{
    this->_size = 0;
    if (this->_digits)
        this->_digits[0] = '\0';
    this->_is_negative = false;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->shrink_capacity();
    return ;
}

void ft_big_number::append_digit_unlocked(char digit) noexcept
{
    ft_size_t required_capacity;
    ft_size_t new_capacity;

    if (digit < '0' || digit > '9')
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    required_capacity = this->_size + 2;
    if (required_capacity > this->_capacity || !this->_digits)
    {
        new_capacity = this->_capacity;
        if (new_capacity < required_capacity)
            new_capacity = required_capacity;
        this->reserve(new_capacity);
        if (ft_big_number::last_operation_error())
            return ;
    }
    if (!this->_digits)
    {
        this->reserve(required_capacity);
        if (ft_big_number::last_operation_error())
            return ;
    }
    if (this->_size == 1 && this->_digits[0] == '0')
        this->_size = 0;
    this->_digits[this->_size] = digit;
    this->_size++;
    this->_digits[this->_size] = '\0';
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_big_number::append_unlocked(const char* digits) noexcept
{
    ft_size_t digit_index;

    if (!digits)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    digit_index = 0;
    while (digits[digit_index] != '\0')
    {
        this->append_digit_unlocked(digits[digit_index]);
        if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
            return ;
        digit_index++;
    }
    return ;
}

void ft_big_number::append_unsigned_unlocked(unsigned long value) noexcept
{
    char digit_buffer[32];
    ft_size_t buffer_index;

    buffer_index = 0;
    if (value == 0)
    {
        digit_buffer[buffer_index] = '0';
        buffer_index++;
    }
    while (value > 0)
    {
        unsigned long remainder;

        remainder = value % 10;
        digit_buffer[buffer_index] = static_cast<char>('0' + remainder);
        buffer_index++;
        value /= 10;
    }
    while (buffer_index > 0)
    {
        buffer_index--;
        this->append_digit_unlocked(digit_buffer[buffer_index]);
        if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
            return ;
    }
    return ;
}

void ft_big_number::trim_leading_zeros_unlocked() noexcept
{
    ft_size_t leading_index;
    ft_size_t new_size;

    if (!this->_digits || this->_size == 0)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
    }
    leading_index = 0;
    while (leading_index + 1 < this->_size && this->_digits[leading_index] == '0')
        leading_index++;
    if (leading_index == 0)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
    }
    new_size = this->_size - leading_index;
    ft_memmove(this->_digits, this->_digits + leading_index, new_size);
    this->_size = new_size;
    this->_digits[this->_size] = '\0';
    this->shrink_capacity();
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_big_number::reduce_to_unlocked(ft_size_t new_size) noexcept
{
    if (new_size >= this->_size)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
    }
    this->_size = new_size;
    if (this->_digits)
    {
        this->_digits[this->_size] = '\0';
        if (this->_size == 0)
            this->_digits[0] = '\0';
    }
    this->shrink_capacity();
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

ft_big_number::ft_big_number() noexcept
    : _digits(ft_nullptr)
    , _size(0)
    , _capacity(0)
    , _is_negative(false)
    , _mutex()
{
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

ft_big_number::ft_big_number(const ft_big_number& other) noexcept
    : _digits(ft_nullptr)
    , _size(0)
    , _capacity(0)
    , _is_negative(false)
    , _mutex()
{
    ft_big_number_mutex_guard other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        this->set_system_error(lock_error);
        return ;
    }
    this->_size = other._size;
    this->_capacity = other._capacity;
    this->_is_negative = other._is_negative;
    if (other._digits && other._capacity > 0)
    {
        this->_digits = static_cast<char*>(cma_calloc(other._capacity, sizeof(char)));
        if (!this->_digits)
        {
            this->_size = 0;
            this->_capacity = 0;
            this->_is_negative = false;
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            this->set_system_error(FT_SYS_ERR_NO_MEMORY);
            return ;
        }
        ft_memcpy(this->_digits, other._digits, other._size + 1);
    }
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

ft_big_number::ft_big_number(ft_big_number&& other) noexcept
    : _digits(ft_nullptr)
    , _size(0)
    , _capacity(0)
    , _is_negative(false)
    , _mutex()
{
    ft_big_number_mutex_guard other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        this->set_system_error(lock_error);
        return ;
    }
    this->_digits = other._digits;
    this->_size = other._size;
    this->_capacity = other._capacity;
    this->_is_negative = other._is_negative;
    other._digits = ft_nullptr;
    other._size = 0;
    other._capacity = 0;
    other._is_negative = false;
    other.set_error_unlocked(FT_ERR_SUCCESSS);
    other.set_system_error(FT_SYS_ERR_SUCCESS);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

ft_big_number::~ft_big_number() noexcept
{
    cma_free(this->_digits);
    return ;
}

ft_big_number& ft_big_number::operator=(const ft_big_number& other) noexcept
{
    ft_big_number_mutex_guard this_guard;
    ft_big_number_mutex_guard other_guard;
    int lock_error;
    char *new_digits;

    if (this == &other)
        return (*this);
    lock_error = ft_big_number::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        this->set_system_error(lock_error);
        return (*this);
    }
    new_digits = ft_nullptr;
    if (other._digits && other._capacity > 0)
    {
        new_digits = static_cast<char*>(cma_calloc(other._capacity, sizeof(char)));
        if (!new_digits)
        {
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            this->set_system_error(FT_SYS_ERR_NO_MEMORY);
            return (*this);
        }
        ft_memcpy(new_digits, other._digits, other._size + 1);
    }
    cma_free(this->_digits);
    this->_digits = new_digits;
    this->_size = other._size;
    this->_capacity = other._capacity;
    this->_is_negative = other._is_negative;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (*this);
}

ft_big_number& ft_big_number::operator=(ft_big_number&& other) noexcept
{
    ft_big_number_mutex_guard this_guard;
    ft_big_number_mutex_guard other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_big_number::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        this->set_system_error(lock_error);
        return (*this);
    }
    cma_free(this->_digits);
    this->_digits = other._digits;
    this->_size = other._size;
    this->_capacity = other._capacity;
    this->_is_negative = other._is_negative;
    other._digits = ft_nullptr;
    other._size = 0;
    other._capacity = 0;
    other._is_negative = false;
    other.set_error_unlocked(FT_ERR_SUCCESSS);
    other.set_system_error(FT_SYS_ERR_SUCCESS);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (*this);
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
        if (other._size > 0 && other._digits)
        {
            result.reserve(other._size + 1);
            if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
                return (result);
            ft_memcpy(result._digits, other._digits, other._size + 1);
            result._size = other._size;
            result._digits[result._size] = '\0';
        }
        else
        {
            result._size = 0;
            if (result._digits)
                result._digits[0] = '\0';
        }
        result._is_negative = false;
        result.set_error_unlocked(FT_ERR_SUCCESSS);
        return (result);
    }
    if (other.is_zero_value())
    {
        if (this->_size > 0 && this->_digits)
        {
            result.reserve(this->_size + 1);
            if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
                return (result);
            ft_memcpy(result._digits, this->_digits, this->_size + 1);
            result._size = this->_size;
            result._digits[result._size] = '\0';
        }
        else
        {
            result._size = 0;
            if (result._digits)
                result._digits[0] = '\0';
        }
        result._is_negative = false;
        result.set_error_unlocked(FT_ERR_SUCCESSS);
        return (result);
    }
    ft_size_t left_length = this->_size;
    ft_size_t right_length = other._size;
    ft_size_t max_length = left_length;
    if (max_length < right_length)
        max_length = right_length;
    max_length++;
    result.reserve(max_length + 1);
    if (ft_big_number::last_operation_error() != 0)
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
        result.set_error_unlocked(FT_ERR_INVALID_STATE);
        return (result);
    }
    if (comparison == 0)
        return (result);
    if (other.is_zero_value())
    {
        if (this->_size > 0 && this->_digits)
        {
            result.reserve(this->_size + 1);
            if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
                return (result);
            ft_memcpy(result._digits, this->_digits, this->_size + 1);
            result._size = this->_size;
            result._digits[result._size] = '\0';
        }
        else
        {
            result._size = 0;
            if (result._digits)
                result._digits[0] = '\0';
        }
        result._is_negative = false;
        result.set_error_unlocked(FT_ERR_SUCCESSS);
        return (result);
    }
    result.reserve(this->_size + 1);
    if (ft_big_number::last_operation_error() != 0)
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
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
    }
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
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        this->set_system_error(FT_SYS_ERR_NO_MEMORY);
        return ;
    }
    this->_digits = new_digits;
    this->_capacity = new_capacity;
    this->set_system_error(FT_SYS_ERR_SUCCESS);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_big_number::shrink_capacity() noexcept
{
    if (!this->_digits)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
    }
    if (this->_size == 0)
    {
        cma_free(this->_digits);
        this->_digits = ft_nullptr;
        this->_capacity = 0;
        this->set_error_unlocked(FT_ERR_SUCCESSS);
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
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
    }
    char* new_digits = static_cast<char*>(cma_realloc(this->_digits, desired_capacity));
    if (!new_digits)
    {
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        this->set_system_error(FT_SYS_ERR_NO_MEMORY);
        return ;
    }
    this->_digits = new_digits;
    this->_capacity = desired_capacity;
    this->set_system_error(FT_SYS_ERR_SUCCESS);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_big_number::assign(const char* number) noexcept
{
    ft_big_number_mutex_guard guard;
    int lock_error;
    bool parsed_negative;
    ft_size_t start_index;
    ft_size_t length;
    ft_size_t index;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
    if (!number)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        guard.unlock();
        return ;
    }
    parsed_negative = false;
    start_index = 0;
    if (number[start_index] == '-')
    {
        parsed_negative = true;
        start_index++;
    }
    length = 0;
    index = start_index;
    while (number[index] != '\0')
    {
        if (number[index] < '0' || number[index] > '9')
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            guard.unlock();
            return ;
        }
        length++;
        index++;
    }
    if (length == 0)
    {
        if (parsed_negative)
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            guard.unlock();
            return ;
        }
        this->clear_unlocked();
        return ;
    }
    if (length + 1 > this->_capacity || !this->_digits)
    {
        this->reserve(length + 1);
        if (!this->_digits)
        {
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            guard.unlock();
            return ;
        }
    }
    ft_memcpy(this->_digits, number + start_index, length);
    this->_digits[length] = '\0';
    this->_size = length;
    this->_is_negative = parsed_negative;
    this->trim_leading_zeros_unlocked();
    if (this->is_zero_value())
        this->_is_negative = false;
    if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS)
        this->shrink_capacity();
    return ;
}

void ft_big_number::assign_base(const char* digits, int base) noexcept
{
    ft_big_number_mutex_guard guard;
    int lock_error;
    int local_error;
    bool parsed_negative;
    ft_size_t index;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    if (!digits)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        guard.unlock();
        return ;
    }
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    local_error = 0;
    parsed_negative = false;
    index = 0;
    if (base < 2 || base > 16)
        local_error = FT_ERR_INVALID_ARGUMENT;
    else
    {
        if (digits[index] == '-')
        {
            parsed_negative = true;
            index++;
        }
        if (digits[index] == '\0')
            local_error = FT_ERR_INVALID_ARGUMENT;
        else
        {
            ft_big_number base_number;

            base_number.append_unsigned(static_cast<unsigned long>(base));
            if (ft_big_number::last_operation_error() != 0)
                local_error = ft_big_number::last_operation_error();
            else
            {
                ft_big_number result;

                while (digits[index] != '\0')
                {
                    int digit_value = ft_big_number_digit_value(digits[index]);

                    if (digit_value < 0 || digit_value >= base)
                    {
                        local_error = FT_ERR_INVALID_ARGUMENT;
                        break;
                    }
                    ft_big_number product = result * base_number;
                    if (ft_big_number::last_operation_error() != 0)
                    {
                        local_error = ft_big_number::last_operation_error();
                        break;
                    }
                    result = product;
                    if (ft_big_number::last_operation_error() != 0)
                    {
                        local_error = ft_big_number::last_operation_error();
                        break;
                    }
                    ft_big_number addend;

                    addend.append_unsigned(static_cast<unsigned long>(digit_value));
                    if (ft_big_number::last_operation_error() != 0)
                    {
                        local_error = ft_big_number::last_operation_error();
                        break;
                    }
                    ft_big_number sum = result + addend;
                    if (ft_big_number::last_operation_error() != 0)
                    {
                        local_error = ft_big_number::last_operation_error();
                        break;
                    }
                    result = sum;
                    if (ft_big_number::last_operation_error() != 0)
                    {
                        local_error = ft_big_number::last_operation_error();
                        break;
                    }
                    index++;
                }
                if (local_error == 0)
                {
                    if (parsed_negative && result.is_zero_value())
                        parsed_negative = false;
                    if (result._size + 1 > this->_capacity || !this->_digits)
                    {
                        this->reserve(result._size + 1);
                        if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
                        {
                            local_error = ft_big_number::last_operation_error();
                        }
                    }
                    if (local_error == 0)
                    {
                        if (result._size > 0 && result._digits)
                            ft_memcpy(this->_digits, result._digits, result._size + 1);
                        else if (this->_digits)
                            this->_digits[0] = '\0';
                        this->_size = result._size;
                        this->_is_negative = parsed_negative;
                        if (this->is_zero_value())
                            this->_is_negative = false;
                        if (this->_digits)
                            this->_digits[this->_size] = '\0';
                        this->shrink_capacity();
                    }
                }
            }
        }
    }
    if (local_error != 0)
    {
        this->set_error_unlocked(local_error);
        guard.unlock();
        return ;
    }
    else
        this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

void ft_big_number::append_digit(char digit) noexcept
{
    ft_big_number_mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    this->append_digit_unlocked(digit);
    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
    {
        int stored_error;

        stored_error = ft_big_number::last_operation_error();
        this->set_error_unlocked(stored_error);
        guard.unlock();
        return ;
    }
    return ;
}

void ft_big_number::append(const char* digits) noexcept
{
    ft_big_number_mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    if (!digits)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        guard.unlock();
        return ;
    }
    this->append_unlocked(digits);
    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
    {
        int stored_error;

        stored_error = ft_big_number::last_operation_error();
        this->set_error_unlocked(stored_error);
        guard.unlock();
        return ;
    }
    return ;
}

void ft_big_number::append_unsigned(unsigned long value) noexcept
{
    ft_big_number_mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    this->append_unsigned_unlocked(value);
    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
    {
        int stored_error;

        stored_error = ft_big_number::last_operation_error();
        this->set_error_unlocked(stored_error);
        guard.unlock();
        return ;
    }
    return ;
}

void ft_big_number::trim_leading_zeros() noexcept
{
    ft_big_number_mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    this->trim_leading_zeros_unlocked();
    return ;
}

void ft_big_number::reduce_to(ft_size_t new_size) noexcept
{
    ft_big_number_mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    this->reduce_to_unlocked(new_size);
    return ;
}

void ft_big_number::clear() noexcept
{
    ft_big_number_mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    this->clear_unlocked();
    return ;
}

ft_big_number ft_big_number::operator+(const ft_big_number& other) const noexcept
{
    ft_big_number::error_scope error_scope;
    ft_big_number_mutex_guard this_guard;
    ft_big_number_mutex_guard other_guard;
    ft_big_number result;
    int lock_error;
    int operation_error;

    operation_error = FT_SYS_ERR_SUCCESS;
    lock_error = ft_big_number::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        operation_error = lock_error;
        result.set_error_unlocked(operation_error);
        result.set_system_error(operation_error);
        goto finalize_add;
    }
    if (ft_big_number::last_operation_error() != 0)
    {
        operation_error = ft_big_number::last_operation_error();
        result.set_error_unlocked(operation_error);
        result.set_system_error(operation_error);
        goto finalize_add;
    }
    if (this->_is_negative == other._is_negative)
    {
        result = this->add_magnitude(other);
        if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS)
        {
            result._is_negative = this->_is_negative;
            if (result.is_zero_value())
                result._is_negative = false;
        }
    }
    else
    {
        int comparison = this->compare_magnitude(other);

        if (comparison == 0)
        {
            return (result);
        }
        if (comparison > 0)
        {
            result = this->subtract_magnitude(other);
            if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS)
                result._is_negative = this->_is_negative;
        }
        else
        {
            result = other.subtract_magnitude(*this);
            if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS)
                result._is_negative = other._is_negative;
        }
        if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS
                && result.is_zero_value())
            result._is_negative = false;
    }
finalize_add:
    if (operation_error == FT_SYS_ERR_SUCCESS
            && ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
    {
        operation_error = ft_big_number::last_operation_error();
        result.set_system_error(operation_error);
    }
    return (result);
}

ft_big_number ft_big_number::operator-(const ft_big_number& other) const noexcept
{
    ft_big_number::error_scope error_scope;
    ft_big_number_mutex_guard this_guard;
    ft_big_number_mutex_guard other_guard;
    ft_big_number result;
    int lock_error;
    int operation_error;
    bool left_negative;
    bool right_negative;

    operation_error = FT_SYS_ERR_SUCCESS;
    lock_error = ft_big_number::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        operation_error = lock_error;
        result.set_error_unlocked(operation_error);
        result.set_system_error(operation_error);
        goto finalize_subtract;
    }
    if (ft_big_number::last_operation_error() != 0)
    {
        operation_error = ft_big_number::last_operation_error();
        result.set_error_unlocked(operation_error);
        result.set_system_error(operation_error);
        goto finalize_subtract;
    }
    left_negative = this->_is_negative;
    right_negative = !other._is_negative;
    if (other.is_zero_value())
        right_negative = false;
    if (left_negative == right_negative)
    {
        result = this->add_magnitude(other);
        if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS)
        {
            result._is_negative = left_negative;
            if (result.is_zero_value())
                result._is_negative = false;
        }
    }
    else
    {
        int comparison = this->compare_magnitude(other);

        if (comparison == 0)
            return (result);
        if (comparison > 0)
        {
            result = this->subtract_magnitude(other);
            if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS)
                result._is_negative = left_negative;
        }
        else
        {
            result = other.subtract_magnitude(*this);
            if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS)
                result._is_negative = right_negative;
        }
        if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS
                && result.is_zero_value())
            result._is_negative = false;
    }
finalize_subtract:
    if (operation_error == FT_SYS_ERR_SUCCESS
            && ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
    {
        operation_error = ft_big_number::last_operation_error();
        result.set_system_error(operation_error);
    }
    return (result);
}

ft_big_number ft_big_number::operator*(const ft_big_number& other) const noexcept
{
    ft_big_number::error_scope error_scope;
    ft_big_number_mutex_guard this_guard;
    ft_big_number_mutex_guard other_guard;
    ft_big_number result;
    int lock_error;
    int operation_error;

    operation_error = FT_SYS_ERR_SUCCESS;
    lock_error = ft_big_number::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        operation_error = lock_error;
        result.set_error_unlocked(operation_error);
        result.set_system_error(operation_error);
        goto finalize_multiply;
    }
    if (ft_big_number::last_operation_error() != 0)
    {
        operation_error = ft_big_number::last_operation_error();
        result.set_error_unlocked(operation_error);
        result.set_system_error(operation_error);
        goto finalize_multiply;
    }
    if (!this->is_zero_value() && !other.is_zero_value())
    {
        ft_size_t left_length = this->_size;
        ft_size_t right_length = other._size;
        ft_size_t result_length = left_length + right_length;

        result.reserve(result_length + 1);
        if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS)
        {
            ft_size_t fill_index = 0;

            while (fill_index < result_length)
            {
                result._digits[fill_index] = '0';
                fill_index++;
            }
            result._digits[result_length] = '\0';
            result._size = result_length;
            ft_size_t left_offset = 0;

            while (left_offset < left_length
                    && ft_big_number::last_operation_error() == FT_ERR_SUCCESSS)
            {
                ft_size_t left_index = left_length - 1 - left_offset;
                int left_digit_value = this->_digits[left_index] - '0';
                int carry_value = 0;
                ft_size_t right_offset = 0;

                while (right_offset < right_length)
                {
                    ft_size_t right_index = right_length - 1 - right_offset;
                    int right_digit_value = other._digits[right_index] - '0';
                    ft_size_t result_position = result_length - 1
                        - (left_offset + right_offset);
                    int existing_value = result._digits[result_position] - '0';
                    int total_value = left_digit_value * right_digit_value
                        + existing_value + carry_value;

                    result._digits[result_position] =
                        static_cast<char>('0' + (total_value % 10));
                    carry_value = total_value / 10;
                    right_offset++;
                }
                ft_size_t carry_position = result_length - 1
                    - (left_offset + right_length);

                while (carry_value > 0)
                {
                    int existing_value = result._digits[carry_position] - '0';
                    int total_value = existing_value + carry_value;

                    result._digits[carry_position] = static_cast<char>('0'
                            + (total_value % 10));
                    carry_value = total_value / 10;
                    if (carry_position == 0)
                        break;
                    carry_position--;
                }
                left_offset++;
            }
            if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS)
            {
                result.trim_leading_zeros();
                if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS
                        && this->_is_negative != other._is_negative)
                    result._is_negative = true;
                if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS
                        && result.is_zero_value())
                    result._is_negative = false;
            }
        }
    }
finalize_multiply:
    if (operation_error == FT_SYS_ERR_SUCCESS
            && ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
    {
        operation_error = ft_big_number::last_operation_error();
        result.set_system_error(operation_error);
    }
    return (result);
}

ft_big_number ft_big_number::operator/(const ft_big_number& other) const noexcept
{
    ft_big_number::error_scope error_scope;
    int stored_errno;
    ft_big_number result;
    ft_big_number_mutex_guard this_guard;
    ft_big_number_mutex_guard other_guard;
    int lock_error;
    int operation_error;
    ft_big_number remainder;
    ft_size_t digit_index;
    int magnitude_comparison;
    int best_digit_value;
    int candidate_digit;

    stored_errno = ft_big_number::initialize_errno_keeper();
    operation_error = FT_ERR_SUCCESSS;
    digit_index = 0;
    magnitude_comparison = 0;
    lock_error = ft_big_number::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        operation_error = lock_error;
        result.set_error_unlocked(operation_error);
        result.set_system_error(operation_error);
        goto cleanup_division;
    }
    if (ft_big_number::last_operation_error() != 0)
    {
        operation_error = ft_big_number::last_operation_error();
        result.set_error_unlocked(operation_error);
        result.set_system_error(operation_error);
        goto cleanup_division;
    }
    if (other.is_zero_value())
    {
        operation_error = FT_ERR_DIVIDE_BY_ZERO;
        result.set_error_unlocked(operation_error);
        result.set_system_error(operation_error);
        goto cleanup_division;
    }
    if (this->is_zero_value())
        goto cleanup_division;
    magnitude_comparison = this->compare_magnitude(other);
    if (magnitude_comparison < 0)
        goto cleanup_division;
    if (magnitude_comparison == 0)
    {
        result.append_digit('1');
        if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS
                && this->_is_negative != other._is_negative)
            result._is_negative = true;
        if (ft_big_number::last_operation_error() == FT_ERR_SUCCESSS
                && result.is_zero_value())
            result._is_negative = false;
        if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
            operation_error = ft_big_number::last_operation_error();
        goto cleanup_division;
    }
    while (digit_index < this->_size)
    {
        remainder.append_digit(this->_digits[digit_index]);
        if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
        {
            operation_error = ft_big_number::last_operation_error();
            goto cleanup_division;
        }
        remainder.trim_leading_zeros();
        if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
        {
            operation_error = ft_big_number::last_operation_error();
            goto cleanup_division;
        }
        best_digit_value = 0;
        if (remainder.compare_magnitude(other) >= 0)
        {
            ft_big_number best_product;

            candidate_digit = 1;
            while (candidate_digit <= 9)
            {
                ft_big_number digit_multiplier;

                digit_multiplier.append_digit(static_cast<char>('0' + candidate_digit));
                if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
                {
                    operation_error = ft_big_number::last_operation_error();
                    goto cleanup_division;
                }
                ft_big_number candidate_product = other * digit_multiplier;

                if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
                {
                    operation_error = ft_big_number::last_operation_error();
                    goto cleanup_division;
                }
                int product_comparison = candidate_product.compare_magnitude(remainder);

                if (product_comparison <= 0)
                {
                    best_digit_value = candidate_digit;
                    best_product = candidate_product;
                    if (product_comparison == 0)
                        candidate_digit = 10;
                    else
                        candidate_digit++;
                }
                else
                    candidate_digit = 10;
            }
            if (best_digit_value > 0)
            {
                remainder = remainder.subtract_magnitude(best_product);
                if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
                {
                    operation_error = ft_big_number::last_operation_error();
                    goto cleanup_division;
                }
            }
        }
        char quotient_digit = static_cast<char>('0' + best_digit_value);

        result.append_digit(quotient_digit);
        if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
        {
            operation_error = ft_big_number::last_operation_error();
            goto cleanup_division;
        }
        digit_index++;
    }
    result.trim_leading_zeros();
    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
        operation_error = ft_big_number::last_operation_error();
    if (operation_error == FT_ERR_SUCCESSS)
    {
        if (this->_is_negative != other._is_negative)
            result._is_negative = true;
        if (result.is_zero_value())
            result._is_negative = false;
    }
cleanup_division:
    if (operation_error != FT_ERR_SUCCESSS)
    {
        result.set_error_unlocked(operation_error);
        ft_big_number::update_errno_keeper(stored_errno, operation_error);
        error_scope.set_error(operation_error);
    }
    ft_big_number::finalize_errno_keeper(stored_errno);
    return (result);
}

ft_big_number ft_big_number::operator%(const ft_big_number& other) const noexcept
{
    ft_big_number::error_scope error_scope;
    int stored_errno;
    ft_big_number result;
    ft_big_number quotient;
    ft_big_number product;
    int operation_error;

    stored_errno = ft_big_number::initialize_errno_keeper();
    operation_error = FT_ERR_SUCCESSS;
    quotient = this->operator/(other);

    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
    {
        operation_error = ft_big_number::last_operation_error();
        result.set_error_unlocked(operation_error);
        result.set_system_error(operation_error);
        goto cleanup_modulus;
    }
    product = quotient * other;

    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
    {
        operation_error = ft_big_number::last_operation_error();
        result.set_error_unlocked(operation_error);
        result.set_system_error(operation_error);
        goto cleanup_modulus;
    }
    result = this->operator-(product);
    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
    {
        operation_error = ft_big_number::last_operation_error();
        result.set_error_unlocked(operation_error);
        result.set_system_error(operation_error);
        goto cleanup_modulus;
    }
    result.trim_leading_zeros();
    if (ft_big_number::last_operation_error() != FT_ERR_SUCCESSS)
    {
        operation_error = ft_big_number::last_operation_error();
        goto cleanup_modulus;
    }
    if (result.is_zero_value())
        result._is_negative = false;
cleanup_modulus:
    if (operation_error != FT_ERR_SUCCESSS)
    {
        result.set_error_unlocked(operation_error);
        ft_big_number::update_errno_keeper(stored_errno, operation_error);
        error_scope.set_error(operation_error);
    }
    ft_big_number::finalize_errno_keeper(stored_errno);
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
    ft_big_number::error_scope error_scope;
    int stored_errno;
    bool result;
    int operation_error;

    stored_errno = ft_big_number::initialize_errno_keeper();
    operation_error = FT_SYS_ERR_SUCCESS;
    {
        ft_big_number_mutex_guard this_guard;
        ft_big_number_mutex_guard other_guard;
        int lock_error;

        lock_error = ft_big_number::lock_pair(*this, other, this_guard, other_guard);
        if (lock_error != FT_ERR_SUCCESSS)
        {
            operation_error = lock_error;
            result = false;
        }
        else if (ft_big_number::last_operation_error() != 0)
        {
            operation_error = ft_big_number::last_operation_error();
            result = false;
        }
        else if (this->is_zero_value() && other.is_zero_value())
            result = false;
        else if (this->_is_negative && !other._is_negative)
            result = true;
        else if (!this->_is_negative && other._is_negative)
            result = false;
        else
        {
            int magnitude_comparison = this->compare_magnitude(other);

            if (this->_is_negative && other._is_negative)
                result = (magnitude_comparison > 0);
            else
                result = (magnitude_comparison < 0);
        }
    }
    if (operation_error != FT_SYS_ERR_SUCCESS)
    {
        this->set_error_unlocked(operation_error);
        ft_big_number::update_errno_keeper(stored_errno, operation_error);
        error_scope.set_error(operation_error);
    }
    else
        this->set_error_unlocked(FT_ERR_SUCCESSS);
    ft_big_number::finalize_errno_keeper(stored_errno);
    return (result);
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
    ft_big_number::error_scope error_scope;
    int stored_errno;
    bool are_equal;
    int operation_error;

    stored_errno = ft_big_number::initialize_errno_keeper();
    operation_error = FT_SYS_ERR_SUCCESS;
    {
        ft_big_number_mutex_guard this_guard;
        ft_big_number_mutex_guard other_guard;
        int lock_error;

        lock_error = ft_big_number::lock_pair(*this, other, this_guard, other_guard);
        if (lock_error != FT_ERR_SUCCESSS)
        {
            operation_error = lock_error;
            are_equal = false;
        }
        else if (ft_big_number::last_operation_error() != 0)
        {
            operation_error = ft_big_number::last_operation_error();
            are_equal = false;
        }
        else if (this->is_zero_value() && other.is_zero_value())
            are_equal = true;
        else if (this->_is_negative != other._is_negative)
            are_equal = false;
        else if (this->_size != other._size)
            are_equal = false;
        else if (this->_size == 0)
            are_equal = true;
        else if (!this->_digits || !other._digits)
            are_equal = false;
        else
        {
            ft_size_t digit_index = 0;

            are_equal = true;
            while (digit_index < this->_size)
            {
                if (this->_digits[digit_index] != other._digits[digit_index])
                {
                    are_equal = false;
                    break;
                }
                digit_index++;
            }
        }
    }
    if (operation_error != FT_SYS_ERR_SUCCESS)
    {
        this->set_error_unlocked(operation_error);
        ft_big_number::update_errno_keeper(stored_errno, operation_error);
        error_scope.set_error(operation_error);
    }
    else
        this->set_error_unlocked(FT_ERR_SUCCESSS);
    ft_big_number::finalize_errno_keeper(stored_errno);
    return (are_equal);
}

const char* ft_big_number::c_str() const noexcept
{
    ft_big_number_mutex_guard guard;
    const char* digits_pointer;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ("0");
    }
    if (this->_digits && this->_size > 0)
        digits_pointer = this->_digits;
    else
        digits_pointer = "0";
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (digits_pointer);
}

ft_size_t ft_big_number::size() const noexcept
{
    ft_big_number_mutex_guard guard;
    ft_size_t current_size;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (0);
    }
    current_size = this->_size;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (current_size);
}

bool ft_big_number::empty() const noexcept
{
    ft_big_number_mutex_guard guard;
    bool is_empty_result;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (true);
    }
    is_empty_result = (this->_size == 0);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (is_empty_result);
}

bool ft_big_number::is_negative() const noexcept
{
    ft_big_number_mutex_guard guard;
    bool result;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (false);
    }
    if (this->is_zero_value())
        result = false;
    else
        result = this->_is_negative;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (result);
}

bool ft_big_number::is_positive() const noexcept
{
    ft_big_number_mutex_guard guard;
    bool result;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (false);
    }
    if (this->is_zero_value())
        result = false;
    else
        result = !this->_is_negative;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (result);
}

ft_string ft_big_number::to_string_base(int base) noexcept
{
    ft_big_number_mutex_guard guard;
    int lock_error;
    bool original_negative;
    bool is_zero_value_result;

    if (base < 2 || base > 16)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (ft_string(FT_ERR_INVALID_ARGUMENT));
    }
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (ft_string(lock_error));
    }
    if (ft_big_number::last_operation_error() != 0)
    {
        int stored_error = ft_big_number::last_operation_error();

        return (ft_string(stored_error));
    }
    is_zero_value_result = this->is_zero_value();
    original_negative = this->_is_negative;
    if (is_zero_value_result)
    {
        ft_string zero_result;

        zero_result.append('0');
        if (ft_big_number::last_operation_error() != 0)
        {
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (ft_string(FT_ERR_NO_MEMORY));
        }
        return (zero_result);
    }
    ft_big_number magnitude(*this);

    if (ft_big_number::last_operation_error() != 0)
    {
        this->set_error_unlocked(ft_big_number::last_operation_error());
        return (ft_string(ft_big_number::last_operation_error()));
    }
    magnitude._is_negative = false;
    ft_big_number base_value;

    base_value.append_unsigned(static_cast<unsigned long>(base));
    if (ft_big_number::last_operation_error() != 0)
    {
        this->set_error_unlocked(ft_big_number::last_operation_error());
        return (ft_string(ft_big_number::last_operation_error()));
    }
    ft_string digits;

    while (!magnitude.is_zero_value())
    {
        ft_big_number quotient = magnitude / base_value;

        if (ft_big_number::last_operation_error() != 0)
        {
            this->set_error_unlocked(ft_big_number::last_operation_error());
            return (ft_string(ft_big_number::last_operation_error()));
        }
        ft_big_number product = quotient * base_value;

        if (ft_big_number::last_operation_error() != 0)
        {
            this->set_error_unlocked(ft_big_number::last_operation_error());
            return (ft_string(ft_big_number::last_operation_error()));
        }
        ft_big_number remainder_number = magnitude - product;

        if (ft_big_number::last_operation_error() != 0)
        {
            this->set_error_unlocked(ft_big_number::last_operation_error());
            return (ft_string(ft_big_number::last_operation_error()));
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
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            return (ft_string(FT_ERR_INVALID_ARGUMENT));
        }
        digits.append(digit_symbol);
        if (ft_big_number::last_operation_error() != 0)
        {
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (ft_string(FT_ERR_NO_MEMORY));
        }
        magnitude = quotient;
        if (ft_big_number::last_operation_error() != 0)
        {
            this->set_error_unlocked(ft_big_number::last_operation_error());
            return (ft_string(ft_big_number::last_operation_error()));
        }
        magnitude._is_negative = false;
    }
    ft_string result;

    if (original_negative)
    {
        result.append('-');
        if (ft_big_number::last_operation_error() != 0)
        {
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (ft_string(FT_ERR_NO_MEMORY));
        }
    }
    const char* digits_buffer = digits.c_str();
    ft_size_t digits_length = digits.size();

    while (digits_length > 0)
    {
        digits_length--;
        result.append(digits_buffer[digits_length]);
        if (ft_big_number::last_operation_error() != 0)
        {
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (ft_string(FT_ERR_NO_MEMORY));
        }
    }
    return (result);
}

ft_big_number ft_big_number::mod_pow(const ft_big_number& exponent, const ft_big_number& modulus) const noexcept
{
    ft_big_number base_value(*this);

    if (ft_big_number::last_operation_error() != 0)
    {
        ft_big_number error_result;

        error_result.set_error_unlocked(ft_big_number::last_operation_error());
        return (error_result);
    }
    ft_big_number exponent_value(exponent);

    if (ft_big_number::last_operation_error() != 0)
    {
        ft_big_number error_result;

        error_result.set_error_unlocked(ft_big_number::last_operation_error());
        return (error_result);
    }
    ft_big_number modulus_value(modulus);

    if (ft_big_number::last_operation_error() != 0)
    {
        ft_big_number error_result;

        error_result.set_error_unlocked(ft_big_number::last_operation_error());
        return (error_result);
    }
    if (modulus_value.is_zero_value())
    {
        ft_big_number error_result;

        error_result.set_error_unlocked(FT_ERR_DIVIDE_BY_ZERO);
        return (error_result);
    }
    if (modulus_value.is_negative())
    {
        ft_big_number error_result;

        error_result.set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (error_result);
    }
    if (exponent_value.is_negative())
    {
        ft_big_number error_result;

        error_result.set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (error_result);
    }
    modulus_value._is_negative = false;
    modulus_value.trim_leading_zeros();
    ft_big_number base_remainder = base_value % modulus_value;

    if (ft_big_number::last_operation_error() != 0)
    {
        ft_big_number error_result;

        error_result.set_error_unlocked(ft_big_number::last_operation_error());
        return (error_result);
    }
    while (base_remainder.is_negative())
    {
        ft_big_number adjusted_base = base_remainder + modulus_value;

        if (ft_big_number::last_operation_error() != 0)
        {
            ft_big_number error_result;

            error_result.set_error_unlocked(ft_big_number::last_operation_error());
            return (error_result);
        }
        base_remainder = adjusted_base;
    }
    base_remainder.trim_leading_zeros();
    exponent_value._is_negative = false;
    exponent_value.trim_leading_zeros();
    ft_big_number one;

    one.append_digit('1');
    if (ft_big_number::last_operation_error() != 0)
    {
        ft_big_number error_result;

        error_result.set_error_unlocked(ft_big_number::last_operation_error());
        return (error_result);
    }
    if (exponent_value.is_zero_value())
    {
        ft_big_number identity_result = one % modulus_value;

        if (ft_big_number::last_operation_error() != 0)
        {
            ft_big_number error_result;

            error_result.set_error_unlocked(ft_big_number::last_operation_error());
            return (error_result);
        }
        identity_result._is_negative = false;
        if (identity_result.is_zero_value())
            identity_result._is_negative = false;
        identity_result.trim_leading_zeros();
        return (identity_result);
    }
    ft_big_number accumulator(one);

    if (ft_big_number::last_operation_error() != 0)
    {
        ft_big_number error_result;

        error_result.set_error_unlocked(ft_big_number::last_operation_error());
        return (error_result);
    }
    while (!exponent_value.is_zero_value())
    {
        ft_big_number product_value = accumulator * base_remainder;

        if (ft_big_number::last_operation_error() != 0)
        {
            ft_big_number error_result;

            error_result.set_error_unlocked(ft_big_number::last_operation_error());
            return (error_result);
        }
        ft_big_number reduced_value = product_value % modulus_value;

        if (ft_big_number::last_operation_error() != 0)
        {
            ft_big_number error_result;

            error_result.set_error_unlocked(ft_big_number::last_operation_error());
            return (error_result);
        }
        accumulator = reduced_value;
        ft_big_number next_exponent = exponent_value - one;

        if (ft_big_number::last_operation_error() != 0)
        {
            ft_big_number error_result;

            error_result.set_error_unlocked(ft_big_number::last_operation_error());
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

const char* ft_big_number::last_operation_error_str() noexcept
{
    const char* error_string;

    error_string = ft_strerror(ft_big_number::last_operation_error());
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

const char* ft_big_number::operation_error_str_at(ft_size_t index) noexcept
{
    const char* error_string;

    error_string = ft_strerror(ft_big_number::operation_error_at(index));
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

void ft_big_number::reset_system_error() const noexcept
{
    ft_big_number_mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        this->set_system_error(lock_error);
        return ;
    }
    this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex* ft_big_number::get_mutex_for_testing() noexcept
{
    return (&(this->_mutex));
}
#endif
