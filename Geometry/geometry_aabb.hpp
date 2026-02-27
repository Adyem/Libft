#ifndef GEOMETRY_AABB_HPP
# define GEOMETRY_AABB_HPP

#include "../PThread/recursive_mutex.hpp"
#include <cstdint>

class aabb
{
    private:
        double                    _minimum_x;
        double                    _minimum_y;
        double                    _maximum_x;
        double                    _maximum_y;
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
        int     lock_pair(const aabb &other, const aabb *&lower,
                    const aabb *&upper) const;
        static void unlock_pair(const aabb *lower, const aabb *upper);

    public:
        aabb();
        aabb(double minimum_x, double minimum_y,
                double maximum_x, double maximum_y);
        aabb(const aabb &other) = delete;
        aabb &operator=(const aabb &other) = delete;
        aabb(aabb &&other) noexcept = delete;
        aabb &operator=(aabb &&other) noexcept = delete;
        ~aabb();

        int     initialize() noexcept;
        int     initialize(double minimum_x, double minimum_y,
                double maximum_x, double maximum_y) noexcept;
        int     initialize(const aabb &other) noexcept;
        int     initialize(aabb &&other) noexcept;
        int     destroy() noexcept;
        int     move(aabb &other) noexcept;

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
        int     enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool    is_thread_safe() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif

        friend bool intersect_aabb(const aabb &first, const aabb &second);
};

#endif
