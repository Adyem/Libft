#ifndef GEOMETRY_SPHERE_HPP
# define GEOMETRY_SPHERE_HPP

#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class sphere
{
    private:
        double          _center_x;
        double          _center_y;
        double          _center_z;
        double          _radius;
        mutable int     _error_code;
        mutable pt_mutex _mutex;

        void    set_error(int error_code) const;
        static int lock_pair(const sphere &first, const sphere &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        sphere();
        sphere(double center_x, double center_y, double center_z, double radius);
        sphere(const sphere &other);
        sphere &operator=(const sphere &other);
        sphere(sphere &&other) noexcept;
        sphere &operator=(sphere &&other) noexcept;
        ~sphere();

        int     set_center(double center_x, double center_y, double center_z);
        int     set_center_x(double center_x);
        int     set_center_y(double center_y);
        int     set_center_z(double center_z);
        int     set_radius(double radius);
        double  get_center_x() const;
        double  get_center_y() const;
        double  get_center_z() const;
        double  get_radius() const;
        int     get_error() const;
        const char  *get_error_str() const;

        friend bool intersect_sphere(const sphere &first, const sphere &second);
};

#endif
