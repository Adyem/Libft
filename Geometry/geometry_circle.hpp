#ifndef GEOMETRY_CIRCLE_HPP
# define GEOMETRY_CIRCLE_HPP

#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class circle
{
    private:
        double          _center_x;
        double          _center_y;
        double          _radius;
        mutable int     _error_code;
        mutable pt_mutex _mutex;

        void    set_error(int error_code) const;
        static int lock_pair(const circle &first, const circle &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

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
        int     get_error() const;
        const char  *get_error_str() const;

        friend bool intersect_circle(const circle &first, const circle &second);
};

#endif
