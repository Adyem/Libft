#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"

#if defined(__SSE2__)
# include <immintrin.h>
#endif

static void matrix2_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int matrix2::lock_mutex() const noexcept
{
    int error;

    if (!this->is_thread_safe_enabled())
        return (FT_ERR_SUCCESSS);
    error = this->_mutex.lock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

int matrix2::unlock_mutex() const noexcept
{
    int error;

    if (!this->is_thread_safe_enabled())
        return (FT_ERR_SUCCESSS);
    error = this->_mutex.unlock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

int matrix2::lock_pair(const matrix2 &first, const matrix2 &second,
    const matrix2 *&lower, const matrix2 *&upper)
{
    const matrix2 *ordered_first;
    const matrix2 *ordered_second;

    if (&first == &second)
    {
        lower = &first;
        upper = &first;
        return (first.lock_mutex());
    }
    ordered_first = &first;
    ordered_second = &second;
    if (ordered_first > ordered_second)
    {
        const matrix2 *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
    }
    lower = ordered_first;
    upper = ordered_second;
    while (true)
    {
        int lower_error = lower->lock_mutex();
        if (lower_error != FT_ERR_SUCCESSS)
        {
            return (lower_error);
        }
        int upper_error = upper->lock_mutex();
        if (upper_error == FT_ERR_SUCCESSS)
        {
            return (FT_ERR_SUCCESSS);
        }
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            lower->unlock_mutex();
            return (upper_error);
        }
        lower->unlock_mutex();
        matrix2_sleep_backoff();
    }
}

void matrix2::unlock_pair(const matrix2 *lower, const matrix2 *upper)
{
    if (upper != ft_nullptr)
        upper->unlock_mutex();
    if (lower != ft_nullptr && lower != upper)
        lower->unlock_mutex();
    return ;
}

matrix2::matrix2(const matrix2 &other)
    : _m(), _mutex()
{
    *this = other;
    return ;
}

matrix2 &matrix2::operator=(const matrix2 &other)
{
    const matrix2 *lower;
    const matrix2 *upper;
    int lock_error;
    int row;
    int column;

    if (this == &other)
        return (*this);
    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    row = 0;
    while (row < 2)
    {
        column = 0;
        while (column < 2)
        {
            this->_m[row][column] = other._m[row][column];
            column++;
        }
        row++;
    }
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

matrix2::matrix2(matrix2 &&other)
    : _m(), _mutex()
{
    *this = ft_move(other);
    return ;
}

matrix2 &matrix2::operator=(matrix2 &&other)
{
    const matrix2 *lower;
    const matrix2 *upper;
    int lock_error;
    int row;
    int column;

    if (this == &other)
        return (*this);
    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    row = 0;
    while (row < 2)
    {
        column = 0;
        while (column < 2)
        {
            this->_m[row][column] = other._m[row][column];
            if (row == column)
                other._m[row][column] = 1.0;
            else
                other._m[row][column] = 0.0;
            column++;
        }
        row++;
    }
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

vector2 matrix2::transform(const vector2 &vector) const
{
    int lock_error;
    vector2 result;
    double vector_x;
    double vector_y;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    vector_x = vector.get_x();
    vector_y = vector.get_y();
    result._x = this->_m[0][0] * vector_x + this->_m[0][1] * vector_y;
    result._y = this->_m[1][0] * vector_x + this->_m[1][1] * vector_y;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        ft_global_error_stack_push(unlock_error);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

matrix2 matrix2::multiply(const matrix2 &other) const
{
    matrix2 result;
    const matrix2 *lower;
    const matrix2 *upper;
    int lock_error;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    result._m[0][0] = this->_m[0][0] * other._m[0][0] + this->_m[0][1] * other._m[1][0];
    result._m[0][1] = this->_m[0][0] * other._m[0][1] + this->_m[0][1] * other._m[1][1];
    result._m[1][0] = this->_m[1][0] * other._m[0][0] + this->_m[1][1] * other._m[1][0];
    result._m[1][1] = this->_m[1][0] * other._m[0][1] + this->_m[1][1] * other._m[1][1];
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

matrix2 matrix2::invert() const
{
    int lock_error;
    double determinant;
    double epsilon;
    matrix2 result;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    determinant = this->_m[0][0] * this->_m[1][1] - this->_m[0][1] * this->_m[1][0];
    epsilon = 0.000001;
    if (math_fabs(determinant) < epsilon)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            ft_global_error_stack_push(unlock_error);
            return (result);
        }
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    result._m[0][0] = this->_m[1][1] / determinant;
    result._m[0][1] = -this->_m[0][1] / determinant;
    result._m[1][0] = -this->_m[1][0] / determinant;
    result._m[1][1] = this->_m[0][0] / determinant;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        ft_global_error_stack_push(unlock_error);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *matrix2::get_mutex_for_testing() noexcept
{
    return (&this->_mutex);
}
#endif

int matrix2::enable_thread_safety() noexcept
{
    this->_thread_safe_enabled = true;
    return (FT_ERR_SUCCESSS);
}

void matrix2::disable_thread_safety() noexcept
{
    this->_thread_safe_enabled = false;
    return ;
}

bool matrix2::is_thread_safe_enabled() const noexcept
{
    return (this->_thread_safe_enabled);
}

static void matrix3_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int matrix3::lock_mutex() const noexcept
{
    int error;

    if (!this->is_thread_safe_enabled())
        return (FT_ERR_SUCCESSS);
    error = this->_mutex.lock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

int matrix3::unlock_mutex() const noexcept
{
    int error;

    if (!this->is_thread_safe_enabled())
        return (FT_ERR_SUCCESSS);
    error = this->_mutex.unlock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

int matrix3::lock_pair(const matrix3 &first, const matrix3 &second,
    const matrix3 *&lower, const matrix3 *&upper)
{
    const matrix3 *ordered_first = &first;
    const matrix3 *ordered_second = &second;

    if (ordered_first == ordered_second)
    {
        lower = &first;
        upper = &first;
        return (first.lock_mutex());
    }
    if (ordered_first > ordered_second)
    {
        const matrix3 *temporary = ordered_first;

        ordered_first = ordered_second;
        ordered_second = temporary;
    }
    lower = ordered_first;
    upper = ordered_second;
    while (true)
    {
        int lower_error = ordered_first->lock_mutex();
        if (lower_error != FT_ERR_SUCCESSS)
            return (lower_error);
        int upper_error = ordered_second->lock_mutex();
        if (upper_error == FT_ERR_SUCCESSS)
            return (FT_ERR_SUCCESSS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ordered_first->unlock_mutex();
            return (upper_error);
        }
        ordered_first->unlock_mutex();
        matrix3_sleep_backoff();
    }
}

void matrix3::unlock_pair(const matrix3 *lower, const matrix3 *upper)
{
    if (upper != ft_nullptr)
        upper->unlock_mutex();
    if (lower != ft_nullptr && lower != upper)
        lower->unlock_mutex();
    return ;
}

matrix3::matrix3(const matrix3 &other)
    : _m(), _mutex()
{
    *this = other;
    return ;
}

matrix3 &matrix3::operator=(const matrix3 &other)
{
    const matrix3 *lower;
    const matrix3 *upper;
    int lock_error;
    int row;
    int column;

    if (this == &other)
        return (*this);
    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    row = 0;
    while (row < 3)
    {
        column = 0;
        while (column < 3)
        {
            this->_m[row][column] = other._m[row][column];
            column++;
        }
        row++;
    }
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

matrix3::matrix3(matrix3 &&other)
    : _m(), _mutex()
{
    *this = ft_move(other);
    return ;
}

matrix3 &matrix3::operator=(matrix3 &&other)
{
    const matrix3 *lower;
    const matrix3 *upper;
    int lock_error;
    int row;
    int column;

    if (this == &other)
        return (*this);
    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    row = 0;
    while (row < 3)
    {
        column = 0;
        while (column < 3)
        {
            this->_m[row][column] = other._m[row][column];
            if (row == column)
                other._m[row][column] = 1.0;
            else
                other._m[row][column] = 0.0;
            column++;
        }
        row++;
    }
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

vector3 matrix3::transform(const vector3 &vector) const
{
    int lock_error;
    vector3 result;
    double vector_x;
    double vector_y;
    double vector_z;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    vector_x = vector.get_x();
    vector_y = vector.get_y();
    vector_z = vector.get_z();
    result._x = this->_m[0][0] * vector_x + this->_m[0][1] * vector_y + this->_m[0][2] * vector_z;
    result._y = this->_m[1][0] * vector_x + this->_m[1][1] * vector_y + this->_m[1][2] * vector_z;
    result._z = this->_m[2][0] * vector_x + this->_m[2][1] * vector_y + this->_m[2][2] * vector_z;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        ft_global_error_stack_push(unlock_error);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

matrix3 matrix3::multiply(const matrix3 &other) const
{
    const matrix3 *lower;
    const matrix3 *upper;
    int lock_error;
    matrix3 result;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    result._m[0][0] = this->_m[0][0] * other._m[0][0] + this->_m[0][1] * other._m[1][0] + this->_m[0][2] * other._m[2][0];
    result._m[0][1] = this->_m[0][0] * other._m[0][1] + this->_m[0][1] * other._m[1][1] + this->_m[0][2] * other._m[2][1];
    result._m[0][2] = this->_m[0][0] * other._m[0][2] + this->_m[0][1] * other._m[1][2] + this->_m[0][2] * other._m[2][2];
    result._m[1][0] = this->_m[1][0] * other._m[0][0] + this->_m[1][1] * other._m[1][0] + this->_m[1][2] * other._m[2][0];
    result._m[1][1] = this->_m[1][0] * other._m[0][1] + this->_m[1][1] * other._m[1][1] + this->_m[1][2] * other._m[2][1];
    result._m[1][2] = this->_m[1][0] * other._m[0][2] + this->_m[1][1] * other._m[1][2] + this->_m[1][2] * other._m[2][2];
    result._m[2][0] = this->_m[2][0] * other._m[0][0] + this->_m[2][1] * other._m[1][0] + this->_m[2][2] * other._m[2][0];
    result._m[2][1] = this->_m[2][0] * other._m[0][1] + this->_m[2][1] * other._m[1][1] + this->_m[2][2] * other._m[2][1];
    result._m[2][2] = this->_m[2][0] * other._m[0][2] + this->_m[2][1] * other._m[1][2] + this->_m[2][2] * other._m[2][2];
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

matrix3 matrix3::invert() const
{
    int lock_error;
    double determinant;
    double inv_det;
    matrix3 result;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    determinant = this->_m[0][0] * (this->_m[1][1] * this->_m[2][2] - this->_m[1][2] * this->_m[2][1])
        - this->_m[0][1] * (this->_m[1][0] * this->_m[2][2] - this->_m[1][2] * this->_m[2][0])
        + this->_m[0][2] * (this->_m[1][0] * this->_m[2][1] - this->_m[1][1] * this->_m[2][0]);
    if (math_absdiff(determinant, 0.0) <= 0.000001)
    {
        unlock_error = this->unlock_mutex();
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            ft_global_error_stack_push(unlock_error);
            return (result);
        }
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (result);
    }
    inv_det = 1.0 / determinant;
    result._m[0][0] = (this->_m[1][1] * this->_m[2][2] - this->_m[1][2] * this->_m[2][1]) * inv_det;
    result._m[0][1] = (this->_m[0][2] * this->_m[2][1] - this->_m[0][1] * this->_m[2][2]) * inv_det;
    result._m[0][2] = (this->_m[0][1] * this->_m[1][2] - this->_m[0][2] * this->_m[1][1]) * inv_det;
    result._m[1][0] = (this->_m[1][2] * this->_m[2][0] - this->_m[1][0] * this->_m[2][2]) * inv_det;
    result._m[1][1] = (this->_m[0][0] * this->_m[2][2] - this->_m[0][2] * this->_m[2][0]) * inv_det;
    result._m[1][2] = (this->_m[0][2] * this->_m[1][0] - this->_m[0][0] * this->_m[1][2]) * inv_det;
    result._m[2][0] = (this->_m[1][0] * this->_m[2][1] - this->_m[1][1] * this->_m[2][0]) * inv_det;
    result._m[2][1] = (this->_m[0][1] * this->_m[2][0] - this->_m[0][0] * this->_m[2][1]) * inv_det;
    result._m[2][2] = (this->_m[0][0] * this->_m[1][1] - this->_m[0][1] * this->_m[1][0]) * inv_det;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        ft_global_error_stack_push(unlock_error);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *matrix3::get_mutex_for_testing() noexcept
{
    return (&this->_mutex);
}
#endif

int matrix3::enable_thread_safety() noexcept
{
    this->_thread_safe_enabled = true;
    return (FT_ERR_SUCCESSS);
}

void matrix3::disable_thread_safety() noexcept
{
    this->_thread_safe_enabled = false;
    return ;
}

bool matrix3::is_thread_safe_enabled() const noexcept
{
    return (this->_thread_safe_enabled);
}

static void matrix4_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int matrix4::lock_mutex() const noexcept
{
    int error;

    if (!this->is_thread_safe_enabled())
        return (FT_ERR_SUCCESSS);
    error = this->_mutex.lock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

int matrix4::unlock_mutex() const noexcept
{
    int error;

    if (!this->is_thread_safe_enabled())
        return (FT_ERR_SUCCESSS);
    error = this->_mutex.unlock(THREAD_ID);
    ft_global_error_stack_pop_newest();
    return (error);
}

#if defined(__SSE2__)
static double matrix4_compute_row_column_simd(const double *row,
    const double *column)
{
    __m128d row_first;
    __m128d column_first;
    __m128d product_first;
    __m128d row_second;
    __m128d column_second;
    __m128d product_second;
    __m128d sum;
    __m128d swapped;
    __m128d total;

    row_first = _mm_set_pd(row[1], row[0]);
    column_first = _mm_set_pd(column[1], column[0]);
    product_first = _mm_mul_pd(row_first, column_first);
    row_second = _mm_set_pd(row[3], row[2]);
    column_second = _mm_set_pd(column[3], column[2]);
    product_second = _mm_mul_pd(row_second, column_second);
    sum = _mm_add_pd(product_first, product_second);
    swapped = _mm_shuffle_pd(sum, sum, 0x1);
    total = _mm_add_sd(sum, swapped);
    return (_mm_cvtsd_f64(total));
}
#else
static double matrix4_compute_row_column_simd(const double *row,
    const double *column)
{
    return (row[0] * column[0] + row[1] * column[1]
        + row[2] * column[2] + row[3] * column[3]);
}
#endif

static double matrix4_compute_row_column(const double *row,
    const double *column)
{
    return (matrix4_compute_row_column_simd(row, column));
}

int matrix4::lock_pair(const matrix4 &first, const matrix4 &second,
    const matrix4 *&lower, const matrix4 *&upper)
{
    const matrix4 *ordered_first = &first;
    const matrix4 *ordered_second = &second;

    if (ordered_first == ordered_second)
    {
        lower = &first;
        upper = &first;
        return (first.lock_mutex());
    }
    if (ordered_first > ordered_second)
    {
        const matrix4 *temporary = ordered_first;

        ordered_first = ordered_second;
        ordered_second = temporary;
    }
    lower = ordered_first;
    upper = ordered_second;
    while (true)
    {
        int lower_error = ordered_first->lock_mutex();
        if (lower_error != FT_ERR_SUCCESSS)
            return (lower_error);
        int upper_error = ordered_second->lock_mutex();
        if (upper_error == FT_ERR_SUCCESSS)
            return (FT_ERR_SUCCESSS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ordered_first->unlock_mutex();
            return (upper_error);
        }
        ordered_first->unlock_mutex();
        matrix4_sleep_backoff();
    }
}

void matrix4::unlock_pair(const matrix4 *lower, const matrix4 *upper)
{
    if (upper != ft_nullptr)
        upper->unlock_mutex();
    if (lower != ft_nullptr && lower != upper)
        lower->unlock_mutex();
    return ;
}

matrix4::matrix4(const matrix4 &other)
    : _m(), _mutex()
{
    *this = other;
    return ;
}

matrix4 &matrix4::operator=(const matrix4 &other)
{
    const matrix4 *lower;
    const matrix4 *upper;
    int lock_error;
    int row;
    int column;

    if (this == &other)
        return (*this);
    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    row = 0;
    while (row < 4)
    {
        column = 0;
        while (column < 4)
        {
            this->_m[row][column] = other._m[row][column];
            column++;
        }
        row++;
    }
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

matrix4::matrix4(matrix4 &&other)
    : _m(), _mutex()
{
    *this = ft_move(other);
    return ;
}

matrix4 &matrix4::operator=(matrix4 &&other)
{
    const matrix4 *lower;
    const matrix4 *upper;
    int lock_error;
    int row;
    int column;

    if (this == &other)
        return (*this);
    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    row = 0;
    while (row < 4)
    {
        column = 0;
        while (column < 4)
        {
            this->_m[row][column] = other._m[row][column];
            if (row == column)
                other._m[row][column] = 1.0;
            else
                other._m[row][column] = 0.0;
            column++;
        }
        row++;
    }
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

vector4 matrix4::transform(const vector4 &vector) const
{
    int lock_error;
    vector4 result;
    double vector_x;
    double vector_y;
    double vector_z;
    double vector_w;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    vector_x = vector.get_x();
    vector_y = vector.get_y();
    vector_z = vector.get_z();
    vector_w = vector.get_w();
    result._x = this->_m[0][0] * vector_x + this->_m[0][1] * vector_y + this->_m[0][2] * vector_z + this->_m[0][3] * vector_w;
    result._y = this->_m[1][0] * vector_x + this->_m[1][1] * vector_y + this->_m[1][2] * vector_z + this->_m[1][3] * vector_w;
    result._z = this->_m[2][0] * vector_x + this->_m[2][1] * vector_y + this->_m[2][2] * vector_z + this->_m[2][3] * vector_w;
    result._w = this->_m[3][0] * vector_x + this->_m[3][1] * vector_y + this->_m[3][2] * vector_z + this->_m[3][3] * vector_w;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        ft_global_error_stack_push(unlock_error);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

matrix4 matrix4::multiply(const matrix4 &other) const
{
    const matrix4 *lower;
    const matrix4 *upper;
    int lock_error;
    matrix4 result;
    size_t column_index;
    size_t row_index;
    double column_values[4];

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    column_index = 0;
    while (column_index < 4)
    {
        column_values[0] = other._m[0][column_index];
        column_values[1] = other._m[1][column_index];
        column_values[2] = other._m[2][column_index];
        column_values[3] = other._m[3][column_index];
        row_index = 0;
        while (row_index < 4)
        {
            result._m[row_index][column_index] = matrix4_compute_row_column(this->_m[row_index], column_values);
            row_index++;
        }
        column_index++;
    }
    this->unlock_pair(lower, upper);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

matrix4 matrix4::invert() const
{
    int lock_error;
    double temp[4][8];
    matrix4 result;
    int row;
    int column;
    int other;
    double pivot;
    double factor;
    int unlock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        ft_global_error_stack_push(lock_error);
        return (result);
    }
    row = 0;
    while (row < 4)
    {
        column = 0;
        while (column < 4)
        {
            temp[row][column] = this->_m[row][column];
            column++;
        }
        column = 0;
        while (column < 4)
        {
            if (row == column)
                temp[row][column + 4] = 1.0;
            else
                temp[row][column + 4] = 0.0;
            column++;
        }
        row++;
    }
    row = 0;
    while (row < 4)
    {
        pivot = temp[row][row];
        if (math_fabs(pivot) < 0.000001)
        {
            unlock_error = this->unlock_mutex();
            if (unlock_error != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(unlock_error);
                ft_global_error_stack_push(unlock_error);
                return (result);
            }
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            return (result);
        }
        column = 0;
        while (column < 8)
        {
            temp[row][column] /= pivot;
            column++;
        }
        other = 0;
        while (other < 4)
        {
            if (other != row)
            {
                factor = temp[other][row];
                column = 0;
                while (column < 8)
                {
                    temp[other][column] -= factor * temp[row][column];
                    column++;
                }
            }
            other++;
        }
        row++;
    }
    row = 0;
    while (row < 4)
    {
        column = 0;
        while (column < 4)
        {
            result._m[row][column] = temp[row][column + 4];
            column++;
        }
        row++;
    }
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        ft_global_error_stack_push(unlock_error);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

matrix4 matrix4::make_translation(double x, double y, double z)
{
    matrix4 result;

    result._m[0][3] = x;
    result._m[1][3] = y;
    result._m[2][3] = z;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

matrix4 matrix4::make_scale(double x, double y, double z)
{
    matrix4 result;

    result._m[0][0] = x;
    result._m[1][1] = y;
    result._m[2][2] = z;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

matrix4 matrix4::make_rotation_x(double angle)
{
    double cos_angle;
    double sin_angle;
    matrix4 result;

    cos_angle = math_cos(angle);
    sin_angle = ft_sin(angle);
    result._m[1][1] = cos_angle;
    result._m[1][2] = -sin_angle;
    result._m[2][1] = sin_angle;
    result._m[2][2] = cos_angle;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

matrix4 matrix4::make_rotation_y(double angle)
{
    double cos_angle;
    double sin_angle;
    matrix4 result;

    cos_angle = math_cos(angle);
    sin_angle = ft_sin(angle);
    result._m[0][0] = cos_angle;
    result._m[0][2] = sin_angle;
    result._m[2][0] = -sin_angle;
    result._m[2][2] = cos_angle;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

matrix4 matrix4::make_rotation_z(double angle)
{
    double cos_angle;
    double sin_angle;
    matrix4 result;

    cos_angle = math_cos(angle);
    sin_angle = ft_sin(angle);
    result._m[0][0] = cos_angle;
    result._m[0][1] = -sin_angle;
    result._m[1][0] = sin_angle;
    result._m[1][1] = cos_angle;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *matrix4::get_mutex_for_testing() noexcept
{
    return (&this->_mutex);
}
#endif

int matrix4::enable_thread_safety() noexcept
{
    this->_thread_safe_enabled = true;
    return (FT_ERR_SUCCESSS);
}

void matrix4::disable_thread_safety() noexcept
{
    this->_thread_safe_enabled = false;
    return ;
}

bool matrix4::is_thread_safe_enabled() const noexcept
{
    return (this->_thread_safe_enabled);
}
