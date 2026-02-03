#ifndef GEOMETRY_SPHERE_HPP
# define GEOMETRY_SPHERE_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"

class sphere
{
    private:
        double          _center_x;
        double          _center_y;
        double          _center_z;
        double          _radius;
        mutable pt_recursive_mutex _mutex;
        mutable bool _thread_safe_enabled = false;

        int     lock_mutex() const noexcept;
        int     unlock_mutex() const noexcept;
        int     lock_pair(const sphere &other, const sphere *&lower,
                const sphere *&upper) const;
        static void unlock_pair(const sphere *lower, const sphere *upper);

    public:
        sphere();
        sphere(double center_x, double center_y, double center_z,
                double radius);
        sphere(const sphere &other);
        sphere &operator=(const sphere &other);
        sphere(sphere &&other) noexcept;
        sphere &operator=(sphere &&other) noexcept;
        ~sphere();

        int     set_center(double center_x, double center_y,
                double center_z);
        int     set_center_x(double center_x);
        int     set_center_y(double center_y);
        int     set_center_z(double center_z);
        int     set_radius(double radius);
        double  get_center_x() const;
        double  get_center_y() const;
        double  get_center_z() const;
        double  get_radius() const;
        int     enable_thread_safety() noexcept;
        void    disable_thread_safety() noexcept;
        bool    is_thread_safe_enabled() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif

        friend bool intersect_sphere(const sphere &first, const sphere &second);
};

#endif
