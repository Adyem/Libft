#ifndef LINEAR_ALGEBRA_QUATERNION_HPP
# define LINEAR_ALGEBRA_QUATERNION_HPP

#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"

class quaternion
{
    private:
        double _w;
        double _x;
        double _y;
        double _z;
        mutable int _error_code;
        mutable pt_recursive_mutex _mutex;
        mutable ft_operation_error_stack _operation_errors;
        void record_operation_error(int error_code) const noexcept;

        void    set_error(int error_code) const;

    public:
        quaternion();
        quaternion(double w, double x, double y, double z);
        quaternion(const quaternion &other);
        quaternion &operator=(const quaternion &other);
        quaternion(quaternion &&other) noexcept;
        quaternion &operator=(quaternion &&other) noexcept;
        ~quaternion();
        double      get_w() const;
        double      get_x() const;
        double      get_y() const;
        double      get_z() const;
        quaternion  add(const quaternion &other) const;
        quaternion  multiply(const quaternion &other) const;
        quaternion  conjugate() const;
        double      length() const;
        quaternion  normalize() const;
        int         get_error() const;
        const char  *get_error_str() const;
        // Low-level error-stack helpers for validation and diagnostics.
        ft_operation_error_stack *get_operation_error_stack_for_validation() noexcept;
        int  last_operation_error() const noexcept;
        const char  *last_operation_error_str() const noexcept;
        int  operation_error_at(ft_size_t index) const noexcept;
        const char  *operation_error_str_at(ft_size_t index) const noexcept;
        void pop_operation_errors() noexcept;
        int  pop_oldest_operation_error() noexcept;
        int  pop_newest_operation_error() noexcept;
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
};

#endif
