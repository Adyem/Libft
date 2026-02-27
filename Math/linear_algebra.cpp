#include "linear_algebra.hpp"
#include "math.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <new>

#if defined(__SSE2__)
# include <immintrin.h>
#endif

static void matrix2_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
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
        return (pt_recursive_mutex_lock_if_not_null(first._mutex));
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
        int lower_error = pt_recursive_mutex_lock_if_not_null(lower->_mutex);
        if (lower_error != FT_ERR_SUCCESS)
        {
            return (lower_error);
        }
        int upper_error = pt_recursive_mutex_lock_if_not_null(upper->_mutex);
        if (upper_error == FT_ERR_SUCCESS)
        {
            return (FT_ERR_SUCCESS);
        }
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
            return (upper_error);
        }
        pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
        matrix2_sleep_backoff();
    }
}

void matrix2::unlock_pair(const matrix2 *lower, const matrix2 *upper)
{
    if (upper != ft_nullptr)
        pt_recursive_mutex_unlock_if_not_null(upper->_mutex);
    if (lower != ft_nullptr && lower != upper)
        pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
    return ;
}

vector2 matrix2::transform(const vector2 &vector) const
{
    int lock_error;
    double vector_x;
    double vector_y;
    double result_x;
    double result_y;
    int unlock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector2());
    }
    vector_x = vector.get_x();
    vector_y = vector.get_y();
    result_x = this->_m[0][0] * vector_x + this->_m[0][1] * vector_y;
    result_y = this->_m[1][0] * vector_x + this->_m[1][1] * vector_y;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (vector2());
    }
    return (vector2(result_x, result_y));
}

matrix2 matrix2::multiply(const matrix2 &other) const
{
    const matrix2 *lower;
    const matrix2 *upper;
    int lock_error;
    double result_00;
    double result_01;
    double result_10;
    double result_11;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (matrix2());
    }
    result_00 = this->_m[0][0] * other._m[0][0] + this->_m[0][1] * other._m[1][0];
    result_01 = this->_m[0][0] * other._m[0][1] + this->_m[0][1] * other._m[1][1];
    result_10 = this->_m[1][0] * other._m[0][0] + this->_m[1][1] * other._m[1][0];
    result_11 = this->_m[1][0] * other._m[0][1] + this->_m[1][1] * other._m[1][1];
    this->unlock_pair(lower, upper);
    return (matrix2(result_00, result_01, result_10, result_11));
}

matrix2 matrix2::invert() const
{
    int lock_error;
    double determinant;
    double epsilon;
    double result_00;
    double result_01;
    double result_10;
    double result_11;
    int unlock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (matrix2());
    }
    determinant = this->_m[0][0] * this->_m[1][1] - this->_m[0][1] * this->_m[1][0];
    epsilon = 0.000001;
    if (math_fabs(determinant) < epsilon)
    {
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            return (matrix2());
        }
        return (matrix2());
    }
    result_00 = this->_m[1][1] / determinant;
    result_01 = -this->_m[0][1] / determinant;
    result_10 = -this->_m[1][0] / determinant;
    result_11 = this->_m[0][0] / determinant;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (matrix2());
    }
    return (matrix2(result_00, result_01, result_10, result_11));
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *matrix2::get_mutex_for_testing() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    if (this->_mutex == ft_nullptr)
    {
        mutex_pointer = new (std::nothrow) pt_recursive_mutex();
        if (mutex_pointer == ft_nullptr)
            return (ft_nullptr);
        mutex_error = mutex_pointer->initialize();
        if (mutex_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            return (ft_nullptr);
        }
        this->_mutex = mutex_pointer;
    }
    return (this->_mutex);
}
#endif

int matrix2::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("matrix2::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int matrix2::disable_thread_safety() noexcept
{
    int mutex_error;

    this->abort_if_not_initialized("matrix2::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        mutex_error = this->_mutex->destroy();
        if (mutex_error != FT_ERR_SUCCESS)
            return (mutex_error);
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return (FT_ERR_SUCCESS);
}

bool matrix2::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("matrix2::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

static void matrix3_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
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
        return (pt_recursive_mutex_lock_if_not_null(first._mutex));
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
        int lower_error = pt_recursive_mutex_lock_if_not_null(ordered_first->_mutex);
        if (lower_error != FT_ERR_SUCCESS)
            return (lower_error);
        int upper_error = pt_recursive_mutex_lock_if_not_null(ordered_second->_mutex);
        if (upper_error == FT_ERR_SUCCESS)
            return (FT_ERR_SUCCESS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            pt_recursive_mutex_unlock_if_not_null(ordered_first->_mutex);
            return (upper_error);
        }
        pt_recursive_mutex_unlock_if_not_null(ordered_first->_mutex);
        matrix3_sleep_backoff();
    }
}

void matrix3::unlock_pair(const matrix3 *lower, const matrix3 *upper)
{
    if (upper != ft_nullptr)
        pt_recursive_mutex_unlock_if_not_null(upper->_mutex);
    if (lower != ft_nullptr && lower != upper)
        pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
    return ;
}

vector3 matrix3::transform(const vector3 &vector) const
{
    int lock_error;
    double vector_x;
    double vector_y;
    double vector_z;
    double result_x;
    double result_y;
    double result_z;
    int unlock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector3());
    }
    vector_x = vector.get_x();
    vector_y = vector.get_y();
    vector_z = vector.get_z();
    result_x = this->_m[0][0] * vector_x + this->_m[0][1] * vector_y + this->_m[0][2] * vector_z;
    result_y = this->_m[1][0] * vector_x + this->_m[1][1] * vector_y + this->_m[1][2] * vector_z;
    result_z = this->_m[2][0] * vector_x + this->_m[2][1] * vector_y + this->_m[2][2] * vector_z;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (vector3());
    }
    return (vector3(result_x, result_y, result_z));
}

matrix3 matrix3::multiply(const matrix3 &other) const
{
    const matrix3 *lower;
    const matrix3 *upper;
    int lock_error;
    double result_00;
    double result_01;
    double result_02;
    double result_10;
    double result_11;
    double result_12;
    double result_20;
    double result_21;
    double result_22;

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (matrix3());
    }
    result_00 = this->_m[0][0] * other._m[0][0] + this->_m[0][1] * other._m[1][0] + this->_m[0][2] * other._m[2][0];
    result_01 = this->_m[0][0] * other._m[0][1] + this->_m[0][1] * other._m[1][1] + this->_m[0][2] * other._m[2][1];
    result_02 = this->_m[0][0] * other._m[0][2] + this->_m[0][1] * other._m[1][2] + this->_m[0][2] * other._m[2][2];
    result_10 = this->_m[1][0] * other._m[0][0] + this->_m[1][1] * other._m[1][0] + this->_m[1][2] * other._m[2][0];
    result_11 = this->_m[1][0] * other._m[0][1] + this->_m[1][1] * other._m[1][1] + this->_m[1][2] * other._m[2][1];
    result_12 = this->_m[1][0] * other._m[0][2] + this->_m[1][1] * other._m[1][2] + this->_m[1][2] * other._m[2][2];
    result_20 = this->_m[2][0] * other._m[0][0] + this->_m[2][1] * other._m[1][0] + this->_m[2][2] * other._m[2][0];
    result_21 = this->_m[2][0] * other._m[0][1] + this->_m[2][1] * other._m[1][1] + this->_m[2][2] * other._m[2][1];
    result_22 = this->_m[2][0] * other._m[0][2] + this->_m[2][1] * other._m[1][2] + this->_m[2][2] * other._m[2][2];
    this->unlock_pair(lower, upper);
    return (matrix3(result_00, result_01, result_02,
            result_10, result_11, result_12,
            result_20, result_21, result_22));
}

matrix3 matrix3::invert() const
{
    int lock_error;
    double determinant;
    double inv_det;
    double result_00;
    double result_01;
    double result_02;
    double result_10;
    double result_11;
    double result_12;
    double result_20;
    double result_21;
    double result_22;
    int unlock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (matrix3());
    }
    determinant = this->_m[0][0] * (this->_m[1][1] * this->_m[2][2] - this->_m[1][2] * this->_m[2][1])
        - this->_m[0][1] * (this->_m[1][0] * this->_m[2][2] - this->_m[1][2] * this->_m[2][0])
        + this->_m[0][2] * (this->_m[1][0] * this->_m[2][1] - this->_m[1][1] * this->_m[2][0]);
    if (math_absdiff(determinant, 0.0) <= 0.000001)
    {
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            return (matrix3());
        }
        return (matrix3());
    }
    inv_det = 1.0 / determinant;
    result_00 = (this->_m[1][1] * this->_m[2][2] - this->_m[1][2] * this->_m[2][1]) * inv_det;
    result_01 = (this->_m[0][2] * this->_m[2][1] - this->_m[0][1] * this->_m[2][2]) * inv_det;
    result_02 = (this->_m[0][1] * this->_m[1][2] - this->_m[0][2] * this->_m[1][1]) * inv_det;
    result_10 = (this->_m[1][2] * this->_m[2][0] - this->_m[1][0] * this->_m[2][2]) * inv_det;
    result_11 = (this->_m[0][0] * this->_m[2][2] - this->_m[0][2] * this->_m[2][0]) * inv_det;
    result_12 = (this->_m[0][2] * this->_m[1][0] - this->_m[0][0] * this->_m[1][2]) * inv_det;
    result_20 = (this->_m[1][0] * this->_m[2][1] - this->_m[1][1] * this->_m[2][0]) * inv_det;
    result_21 = (this->_m[0][1] * this->_m[2][0] - this->_m[0][0] * this->_m[2][1]) * inv_det;
    result_22 = (this->_m[0][0] * this->_m[1][1] - this->_m[0][1] * this->_m[1][0]) * inv_det;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (matrix3());
    }
    return (matrix3(result_00, result_01, result_02,
            result_10, result_11, result_12,
            result_20, result_21, result_22));
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *matrix3::get_mutex_for_testing() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    if (this->_mutex == ft_nullptr)
    {
        mutex_pointer = new (std::nothrow) pt_recursive_mutex();
        if (mutex_pointer == ft_nullptr)
            return (ft_nullptr);
        mutex_error = mutex_pointer->initialize();
        if (mutex_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            return (ft_nullptr);
        }
        this->_mutex = mutex_pointer;
    }
    return (this->_mutex);
}
#endif

int matrix3::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("matrix3::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int matrix3::disable_thread_safety() noexcept
{
    int mutex_error;

    this->abort_if_not_initialized("matrix3::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        mutex_error = this->_mutex->destroy();
        if (mutex_error != FT_ERR_SUCCESS)
            return (mutex_error);
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return (FT_ERR_SUCCESS);
}

bool matrix3::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("matrix3::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

static void matrix4_sleep_backoff()
{
    pt_thread_sleep(1);
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

int matrix4::lock_pair(const matrix4 &first, const matrix4 &second,
    const matrix4 *&lower, const matrix4 *&upper)
{
    const matrix4 *ordered_first = &first;
    const matrix4 *ordered_second = &second;

    if (ordered_first == ordered_second)
    {
        lower = &first;
        upper = &first;
        return (pt_recursive_mutex_lock_if_not_null(first._mutex));
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
        int lower_error = pt_recursive_mutex_lock_if_not_null(ordered_first->_mutex);
        if (lower_error != FT_ERR_SUCCESS)
            return (lower_error);
        int upper_error = pt_recursive_mutex_lock_if_not_null(ordered_second->_mutex);
        if (upper_error == FT_ERR_SUCCESS)
            return (FT_ERR_SUCCESS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            pt_recursive_mutex_unlock_if_not_null(ordered_first->_mutex);
            return (upper_error);
        }
        pt_recursive_mutex_unlock_if_not_null(ordered_first->_mutex);
        matrix4_sleep_backoff();
    }
}

void matrix4::unlock_pair(const matrix4 *lower, const matrix4 *upper)
{
    if (upper != ft_nullptr)
        pt_recursive_mutex_unlock_if_not_null(upper->_mutex);
    if (lower != ft_nullptr && lower != upper)
        pt_recursive_mutex_unlock_if_not_null(lower->_mutex);
    return ;
}

vector4 matrix4::transform(const vector4 &vector) const
{
    int lock_error;
    double vector_x;
    double vector_y;
    double vector_z;
    double vector_w;
    double result_x;
    double result_y;
    double result_z;
    double result_w;
    int unlock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (vector4());
    }
    vector_x = vector.get_x();
    vector_y = vector.get_y();
    vector_z = vector.get_z();
    vector_w = vector.get_w();
    result_x = this->_m[0][0] * vector_x + this->_m[0][1] * vector_y + this->_m[0][2] * vector_z + this->_m[0][3] * vector_w;
    result_y = this->_m[1][0] * vector_x + this->_m[1][1] * vector_y + this->_m[1][2] * vector_z + this->_m[1][3] * vector_w;
    result_z = this->_m[2][0] * vector_x + this->_m[2][1] * vector_y + this->_m[2][2] * vector_z + this->_m[2][3] * vector_w;
    result_w = this->_m[3][0] * vector_x + this->_m[3][1] * vector_y + this->_m[3][2] * vector_z + this->_m[3][3] * vector_w;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (vector4());
    }
    return (vector4(result_x, result_y, result_z, result_w));
}

matrix4 matrix4::multiply(const matrix4 &other) const
{
    const matrix4 *lower;
    const matrix4 *upper;
    int lock_error;
    matrix4 result_matrix;
    size_t column_index;
    size_t row_index;
    double column_values[4];

    lock_error = this->lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (matrix4());
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
            result_matrix._m[row_index][column_index] = matrix4_compute_row_column(this->_m[row_index], column_values);
            row_index++;
        }
        column_index++;
    }
    this->unlock_pair(lower, upper);
    return (matrix4(result_matrix._m[0][0], result_matrix._m[0][1],
            result_matrix._m[0][2], result_matrix._m[0][3],
            result_matrix._m[1][0], result_matrix._m[1][1],
            result_matrix._m[1][2], result_matrix._m[1][3],
            result_matrix._m[2][0], result_matrix._m[2][1],
            result_matrix._m[2][2], result_matrix._m[2][3],
            result_matrix._m[3][0], result_matrix._m[3][1],
            result_matrix._m[3][2], result_matrix._m[3][3]));
}

matrix4 matrix4::invert() const
{
    int lock_error;
    double temp[4][8];
    matrix4 result_matrix;
    int row;
    int column;
    int other;
    double pivot;
    double factor;
    int unlock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (matrix4());
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
            unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                return (matrix4());
            }
            return (matrix4());
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
            result_matrix._m[row][column] = temp[row][column + 4];
            column++;
        }
        row++;
    }
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (matrix4());
    }
    return (matrix4(result_matrix._m[0][0], result_matrix._m[0][1],
            result_matrix._m[0][2], result_matrix._m[0][3],
            result_matrix._m[1][0], result_matrix._m[1][1],
            result_matrix._m[1][2], result_matrix._m[1][3],
            result_matrix._m[2][0], result_matrix._m[2][1],
            result_matrix._m[2][2], result_matrix._m[2][3],
            result_matrix._m[3][0], result_matrix._m[3][1],
            result_matrix._m[3][2], result_matrix._m[3][3]));
}

matrix4 matrix4::make_translation(double x, double y, double z)
{
    return (matrix4(1.0, 0.0, 0.0, x,
            0.0, 1.0, 0.0, y,
            0.0, 0.0, 1.0, z,
            0.0, 0.0, 0.0, 1.0));
}

matrix4 matrix4::make_scale(double x, double y, double z)
{
    return (matrix4(x, 0.0, 0.0, 0.0,
            0.0, y, 0.0, 0.0,
            0.0, 0.0, z, 0.0,
            0.0, 0.0, 0.0, 1.0));
}

matrix4 matrix4::make_rotation_x(double angle)
{
    double cos_angle;
    double sin_angle;

    cos_angle = math_cos(angle);
    sin_angle = ft_sin(angle);
    return (matrix4(1.0, 0.0, 0.0, 0.0,
            0.0, cos_angle, -sin_angle, 0.0,
            0.0, sin_angle, cos_angle, 0.0,
            0.0, 0.0, 0.0, 1.0));
}

matrix4 matrix4::make_rotation_y(double angle)
{
    double cos_angle;
    double sin_angle;

    cos_angle = math_cos(angle);
    sin_angle = ft_sin(angle);
    return (matrix4(cos_angle, 0.0, sin_angle, 0.0,
            0.0, 1.0, 0.0, 0.0,
            -sin_angle, 0.0, cos_angle, 0.0,
            0.0, 0.0, 0.0, 1.0));
}

matrix4 matrix4::make_rotation_z(double angle)
{
    double cos_angle;
    double sin_angle;

    cos_angle = math_cos(angle);
    sin_angle = ft_sin(angle);
    return (matrix4(cos_angle, -sin_angle, 0.0, 0.0,
            sin_angle, cos_angle, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0));
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *matrix4::get_mutex_for_testing() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    if (this->_mutex == ft_nullptr)
    {
        mutex_pointer = new (std::nothrow) pt_recursive_mutex();
        if (mutex_pointer == ft_nullptr)
            return (ft_nullptr);
        mutex_error = mutex_pointer->initialize();
        if (mutex_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            return (ft_nullptr);
        }
        this->_mutex = mutex_pointer;
    }
    return (this->_mutex);
}
#endif

int matrix4::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("matrix4::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int matrix4::disable_thread_safety() noexcept
{
    int mutex_error;

    this->abort_if_not_initialized("matrix4::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        mutex_error = this->_mutex->destroy();
        if (mutex_error != FT_ERR_SUCCESS)
            return (mutex_error);
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return (FT_ERR_SUCCESS);
}

bool matrix4::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("matrix4::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}
