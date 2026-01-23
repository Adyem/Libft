#ifndef GEOMETRY_AABB_HPP
# define GEOMETRY_AABB_HPP

#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"

class aabb
{
    private:
        double          _minimum_x;
        double          _minimum_y;
        double          _maximum_x;
        double          _maximum_y;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};
        mutable pt_recursive_mutex _mutex;

        void    record_operation_error(int error_code) const noexcept;
        int     lock_pair(const aabb &other, const aabb *&lower,
                const aabb *&upper) const;
        static void unlock_pair(const aabb *lower, const aabb *upper);

    public:
        aabb();
        aabb(double minimum_x, double minimum_y,
                double maximum_x, double maximum_y);
        aabb(const aabb &other);
        aabb &operator=(const aabb &other);
        aabb(aabb &&other) noexcept;
        aabb &operator=(aabb &&other) noexcept;
        ~aabb();

        int     set_bounds(double minimum_x, double minimum_y,
                double maximum_x, double maximum_y);
        int     set_minimum(double minimum_x, double minimum_y);
        int     set_minimum_x(double minimum_x);
        int     set_minimum_y(double minimum_y);
        int     set_maximum(double maximum_x, double maximum_y);
        int     set_maximum_x(double maximum_x);
        int     set_maximum_y(double maximum_y);
        double  get_minimum_x() const;
        double  get_minimum_y() const;
        double  get_maximum_x() const;
        double  get_maximum_y() const;
        pt_recursive_mutex *get_mutex_for_validation() const;
        ft_operation_error_stack *get_operation_error_stack_for_validation() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif

        friend bool intersect_aabb(const aabb &first, const aabb &second);
};

#endif
