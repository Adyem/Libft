#ifndef GEOMETRY_CIRCLE_HPP
# define GEOMETRY_CIRCLE_HPP

#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"

class circle
{
    private:
        double          _center_x;
        double          _center_y;
        double          _radius;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};
        mutable pt_recursive_mutex _mutex;

        void    record_operation_error(int error_code) const noexcept;
        int     lock_pair(const circle &other, const circle *&lower,
                const circle *&upper) const;
        static void unlock_pair(const circle *lower, const circle *upper);

    public:
        circle();
        circle(double center_x, double center_y, double radius);
        circle(const circle &other);
        circle &operator=(const circle &other);
        circle(circle &&other) noexcept;
        circle &operator=(circle &&other) noexcept;
        ~circle();

        int     set_center(double center_x, double center_y);
        int     set_center_x(double center_x);
        int     set_center_y(double center_y);
        int     set_radius(double radius);
        double  get_center_x() const;
        double  get_center_y() const;
        double  get_radius() const;
        pt_recursive_mutex *get_mutex_for_validation() const;
        ft_operation_error_stack *get_operation_error_stack_for_validation() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif

        friend bool intersect_circle(const circle &first, const circle &second);
};

#endif
