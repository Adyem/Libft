#ifndef GEOMETRY_SPHERE_HPP
# define GEOMETRY_SPHERE_HPP

#include "../PThread/recursive_mutex.hpp"
#include <cstdint>

class sphere
{
    private:
        double                    _center_x;
        double                    _center_y;
        double                    _center_z;
        double                    _radius;
        mutable pt_recursive_mutex *_mutex;
        uint8_t                   _initialized_state;
        static const uint8_t      _state_uninitialized = 0;
        static const uint8_t      _state_destroyed = 1;
        static const uint8_t      _state_initialized = 2;

        void    abort_lifecycle_error(const char *method_name,
                    const char *reason) const noexcept;
        void    abort_if_not_initialized(const char *method_name) const noexcept;
        int     lock_mutex() const noexcept;
        int     unlock_mutex() const noexcept;
        int     lock_pair(const sphere &other, const sphere *&lower,
                    const sphere *&upper) const;
        static void unlock_pair(const sphere *lower, const sphere *upper);

    public:
        sphere();
        sphere(double center_x, double center_y, double center_z,
                double radius);
        sphere(const sphere &other) = delete;
        sphere &operator=(const sphere &other) = delete;
        sphere(sphere &&other) noexcept = delete;
        sphere &operator=(sphere &&other) noexcept = delete;
        ~sphere();

        int     initialize() noexcept;
        int     initialize(double center_x, double center_y, double center_z,
                double radius) noexcept;
        int     initialize(const sphere &other) noexcept;
        int     initialize(sphere &&other) noexcept;
        int     destroy() noexcept;
        int     move(sphere &other) noexcept;

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
