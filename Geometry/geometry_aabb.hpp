#ifndef GEOMETRY_AABB_HPP
# define GEOMETRY_AABB_HPP

#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class aabb
{
    private:
        double          _minimum_x;
        double          _minimum_y;
        double          _maximum_x;
        double          _maximum_y;
        mutable int     _error_code;
        mutable pt_mutex _mutex;

        void    set_error(int error_code) const;
        static int lock_pair(const aabb &first, const aabb &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

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
        int     get_error() const;
        const char  *get_error_str() const;

        friend bool intersect_aabb(const aabb &first, const aabb &second);
};

#endif
