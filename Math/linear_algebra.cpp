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

static void matrix2_restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int matrix2::lock_self(ft_unique_lock<pt_mutex> &guard) const
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);
    if (local_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (ER_SUCCESS);
}

int matrix2::lock_pair(const matrix2 &first, const matrix2 &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const matrix2 *ordered_first;
    const matrix2 *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const matrix2 *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
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
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        matrix2_sleep_backoff();
    }
}

void matrix2::set_error_unlocked(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void matrix2::set_error(int error_code) const
{
    this->set_error_unlocked(error_code);
    return ;
}

matrix2::matrix2(const matrix2 &other)
    : _m(), _error_code(ER_SUCCESS), _mutex()
{
    *this = other;
    return ;
}

matrix2 &matrix2::operator=(const matrix2 &other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    int row;
    int column;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = matrix2::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
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
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    matrix2_restore_errno(this_guard, entry_errno);
    matrix2_restore_errno(other_guard, entry_errno);
    return (*this);
}

matrix2::matrix2(matrix2 &&other)
    : _m(), _error_code(ER_SUCCESS), _mutex()
{
    *this = ft_move(other);
    return ;
}

matrix2 &matrix2::operator=(matrix2 &&other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    int row;
    int column;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = matrix2::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
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
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    other._error_code = ER_SUCCESS;
    other.set_error_unlocked(ER_SUCCESS);
    matrix2_restore_errno(this_guard, entry_errno);
    matrix2_restore_errno(other_guard, entry_errno);
    return (*this);
}

vector2 matrix2::transform(const vector2 &vector) const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    vector2 result;
    double vector_x;
    double vector_y;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    vector_x = vector.get_x();
    vector_y = vector.get_y();
    result._x = this->_m[0][0] * vector_x + this->_m[0][1] * vector_y;
    result._y = this->_m[1][0] * vector_x + this->_m[1][1] * vector_y;
    result.set_error_unlocked(ER_SUCCESS);
    this->set_error_unlocked(ER_SUCCESS);
    matrix2_restore_errno(guard, entry_errno);
    return (result);
}

matrix2 matrix2::multiply(const matrix2 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    matrix2 result;

    entry_errno = ft_errno;
    lock_error = matrix2::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    result._m[0][0] = this->_m[0][0] * other._m[0][0] + this->_m[0][1] * other._m[1][0];
    result._m[0][1] = this->_m[0][0] * other._m[0][1] + this->_m[0][1] * other._m[1][1];
    result._m[1][0] = this->_m[1][0] * other._m[0][0] + this->_m[1][1] * other._m[1][0];
    result._m[1][1] = this->_m[1][0] * other._m[0][1] + this->_m[1][1] * other._m[1][1];
    result.set_error_unlocked(ER_SUCCESS);
    this->set_error_unlocked(ER_SUCCESS);
    matrix2_restore_errno(this_guard, entry_errno);
    matrix2_restore_errno(other_guard, entry_errno);
    return (result);
}

matrix2 matrix2::invert() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    double determinant;
    double epsilon;
    matrix2 result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    determinant = this->_m[0][0] * this->_m[1][1] - this->_m[0][1] * this->_m[1][0];
    epsilon = 0.000001;
    if (math_fabs(determinant) < epsilon)
    {
        result.set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        matrix2_restore_errno(guard, entry_errno);
        return (result);
    }
    result._m[0][0] = this->_m[1][1] / determinant;
    result._m[0][1] = -this->_m[0][1] / determinant;
    result._m[1][0] = -this->_m[1][0] / determinant;
    result._m[1][1] = this->_m[0][0] / determinant;
    result.set_error_unlocked(ER_SUCCESS);
    this->set_error_unlocked(ER_SUCCESS);
    matrix2_restore_errno(guard, entry_errno);
    return (result);
}

int matrix2::get_error() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int error_value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (lock_error);
    }
    error_value = this->_error_code;
    matrix2_restore_errno(guard, entry_errno);
    return (error_value);
}

const char *matrix2::get_error_str() const
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}

static void matrix3_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void matrix3_restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int matrix3::lock_self(ft_unique_lock<pt_mutex> &guard) const
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);
    if (local_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (ER_SUCCESS);
}

int matrix3::lock_pair(const matrix3 &first, const matrix3 &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const matrix3 *ordered_first;
    const matrix3 *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const matrix3 *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
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
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        matrix3_sleep_backoff();
    }
}

void matrix3::set_error_unlocked(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void matrix3::set_error(int error_code) const
{
    this->set_error_unlocked(error_code);
    return ;
}

matrix3::matrix3(const matrix3 &other)
    : _m(), _error_code(ER_SUCCESS), _mutex()
{
    *this = other;
    return ;
}

matrix3 &matrix3::operator=(const matrix3 &other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    int row;
    int column;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = matrix3::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
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
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    matrix3_restore_errno(this_guard, entry_errno);
    matrix3_restore_errno(other_guard, entry_errno);
    return (*this);
}

matrix3::matrix3(matrix3 &&other)
    : _m(), _error_code(ER_SUCCESS), _mutex()
{
    *this = ft_move(other);
    return ;
}

matrix3 &matrix3::operator=(matrix3 &&other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    int row;
    int column;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = matrix3::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
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
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    other._error_code = ER_SUCCESS;
    other.set_error_unlocked(ER_SUCCESS);
    matrix3_restore_errno(this_guard, entry_errno);
    matrix3_restore_errno(other_guard, entry_errno);
    return (*this);
}

vector3 matrix3::transform(const vector3 &vector) const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    vector3 result;
    double vector_x;
    double vector_y;
    double vector_z;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    vector_x = vector.get_x();
    vector_y = vector.get_y();
    vector_z = vector.get_z();
    result._x = this->_m[0][0] * vector_x + this->_m[0][1] * vector_y + this->_m[0][2] * vector_z;
    result._y = this->_m[1][0] * vector_x + this->_m[1][1] * vector_y + this->_m[1][2] * vector_z;
    result._z = this->_m[2][0] * vector_x + this->_m[2][1] * vector_y + this->_m[2][2] * vector_z;
    result.set_error_unlocked(ER_SUCCESS);
    this->set_error_unlocked(ER_SUCCESS);
    matrix3_restore_errno(guard, entry_errno);
    return (result);
}

matrix3 matrix3::multiply(const matrix3 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    matrix3 result;

    entry_errno = ft_errno;
    lock_error = matrix3::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
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
    result.set_error_unlocked(ER_SUCCESS);
    this->set_error_unlocked(ER_SUCCESS);
    matrix3_restore_errno(this_guard, entry_errno);
    matrix3_restore_errno(other_guard, entry_errno);
    return (result);
}

matrix3 matrix3::invert() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    double determinant;
    double inv_det;
    matrix3 result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    determinant = this->_m[0][0] * (this->_m[1][1] * this->_m[2][2] - this->_m[1][2] * this->_m[2][1])
        - this->_m[0][1] * (this->_m[1][0] * this->_m[2][2] - this->_m[1][2] * this->_m[2][0])
        + this->_m[0][2] * (this->_m[1][0] * this->_m[2][1] - this->_m[1][1] * this->_m[2][0]);
    if (math_absdiff(determinant, 0.0) <= 0.000001)
    {
        result.set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        matrix3_restore_errno(guard, entry_errno);
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
    result.set_error_unlocked(ER_SUCCESS);
    this->set_error_unlocked(ER_SUCCESS);
    matrix3_restore_errno(guard, entry_errno);
    return (result);
}

int matrix3::get_error() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int error_value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (lock_error);
    }
    error_value = this->_error_code;
    matrix3_restore_errno(guard, entry_errno);
    return (error_value);
}

const char *matrix3::get_error_str() const
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}

static void matrix4_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void matrix4_restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
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

int matrix4::lock_self(ft_unique_lock<pt_mutex> &guard) const
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);
    if (local_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (ER_SUCCESS);
}

int matrix4::lock_pair(const matrix4 &first, const matrix4 &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const matrix4 *ordered_first;
    const matrix4 *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const matrix4 *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
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
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        matrix4_sleep_backoff();
    }
}

void matrix4::set_error_unlocked(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void matrix4::set_error(int error_code) const
{
    this->set_error_unlocked(error_code);
    return ;
}

matrix4::matrix4(const matrix4 &other)
    : _m(), _error_code(ER_SUCCESS), _mutex()
{
    *this = other;
    return ;
}

matrix4 &matrix4::operator=(const matrix4 &other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    int row;
    int column;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = matrix4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
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
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    matrix4_restore_errno(this_guard, entry_errno);
    matrix4_restore_errno(other_guard, entry_errno);
    return (*this);
}

matrix4::matrix4(matrix4 &&other)
    : _m(), _error_code(ER_SUCCESS), _mutex()
{
    *this = ft_move(other);
    return ;
}

matrix4 &matrix4::operator=(matrix4 &&other)
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    int row;
    int column;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = matrix4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
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
    this->_error_code = other._error_code;
    this->set_error_unlocked(other._error_code);
    other._error_code = ER_SUCCESS;
    other.set_error_unlocked(ER_SUCCESS);
    matrix4_restore_errno(this_guard, entry_errno);
    matrix4_restore_errno(other_guard, entry_errno);
    return (*this);
}

vector4 matrix4::transform(const vector4 &vector) const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    vector4 result;
    double vector_x;
    double vector_y;
    double vector_z;
    double vector_w;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
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
    result.set_error_unlocked(ER_SUCCESS);
    this->set_error_unlocked(ER_SUCCESS);
    matrix4_restore_errno(guard, entry_errno);
    return (result);
}

matrix4 matrix4::multiply(const matrix4 &other) const
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    matrix4 result;

    entry_errno = ft_errno;
    lock_error = matrix4::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
        return (result);
    }
    size_t column_index;
    size_t row_index;
    double column_values[4];

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
    result.set_error_unlocked(ER_SUCCESS);
    this->set_error_unlocked(ER_SUCCESS);
    matrix4_restore_errno(this_guard, entry_errno);
    matrix4_restore_errno(other_guard, entry_errno);
    return (result);
}

matrix4 matrix4::invert() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    double temp[4][8];
    matrix4 result;
    int row;
    int column;
    int other;
    double pivot;
    double factor;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        result.set_error_unlocked(lock_error);
        this->set_error_unlocked(lock_error);
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
            result.set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            matrix4_restore_errno(guard, entry_errno);
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
    result.set_error_unlocked(ER_SUCCESS);
    this->set_error_unlocked(ER_SUCCESS);
    matrix4_restore_errno(guard, entry_errno);
    return (result);
}

matrix4 matrix4::make_translation(double x, double y, double z)
{
    matrix4 result;

    result._m[0][3] = x;
    result._m[1][3] = y;
    result._m[2][3] = z;
    result.set_error_unlocked(ER_SUCCESS);
    return (result);
}

matrix4 matrix4::make_scale(double x, double y, double z)
{
    matrix4 result;

    result._m[0][0] = x;
    result._m[1][1] = y;
    result._m[2][2] = z;
    result.set_error_unlocked(ER_SUCCESS);
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
    result.set_error_unlocked(ER_SUCCESS);
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
    result.set_error_unlocked(ER_SUCCESS);
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
    result.set_error_unlocked(ER_SUCCESS);
    return (result);
}

int matrix4::get_error() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int error_value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (lock_error);
    }
    error_value = this->_error_code;
    matrix4_restore_errno(guard, entry_errno);
    return (error_value);
}

const char *matrix4::get_error_str() const
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}

