#ifndef GEOMETRY_CIRCLE_HPP
# define GEOMETRY_CIRCLE_HPP

#include "../PThread/recursive_mutex.hpp"
#include <cstdint>

class circle
{
    private:
        double                    _center_x;
        double                    _center_y;
        double                    _radius;
        mutable pt_recursive_mutex *_mutex;
        uint8_t                   _initialized_state;
        static const uint8_t      _state_uninitialized = 0;
        static const uint8_t      _state_destroyed = 1;
        static const uint8_t      _state_initialized = 2;

        void    abort_lifecycle_error(const char *method_name,
                    const char *reason) const noexcept;
        void    abort_if_not_initialized(const char *method_name) const noexcept;
        int     lock_pair(const circle &other, const circle *&lower,
                    const circle *&upper) const;
        static void unlock_pair(const circle *lower, const circle *upper);

    public:
        circle();
        circle(double center_x, double center_y, double radius);
        circle(const circle &other) = delete;
        circle &operator=(const circle &other) = delete;
        circle(circle &&other) noexcept = delete;
        circle &operator=(circle &&other) noexcept = delete;
        ~circle();

        int     initialize() noexcept;
        int     initialize(double center_x, double center_y, double radius) noexcept;
        int     initialize(const circle &other) noexcept;
        int     initialize(circle &&other) noexcept;
        int     destroy() noexcept;
        int     move(circle &other) noexcept;

        int     set_center(double center_x, double center_y);
        int     set_center_x(double center_x);
        int     set_center_y(double center_y);
        int     set_radius(double radius);
        double  get_center_x() const;
        double  get_center_y() const;
        double  get_radius() const;
        int     enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool    is_thread_safe() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif

        friend bool intersect_circle(const circle &first, const circle &second);
};

#endif
