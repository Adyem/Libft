#ifndef LINEAR_ALGEBRA_QUATERNION_HPP
# define LINEAR_ALGEBRA_QUATERNION_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdint>

class quaternion
{
    private:
        double _w;
        double _x;
        double _y;
        double _z;
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialized_state = 0;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        static int lock_pair(const quaternion &first, const quaternion &second,
                const quaternion *&lower, const quaternion *&upper);
        static void unlock_pair(const quaternion *lower, const quaternion *upper);
        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;

    protected:
        int lock_mutex() const noexcept;
        int unlock_mutex() const noexcept;

    public:
        quaternion();
        quaternion(double w, double x, double y, double z);
        quaternion(const quaternion &other) = delete;
        quaternion &operator=(const quaternion &other) = delete;
        quaternion(quaternion &&other) noexcept = delete;
        quaternion &operator=(quaternion &&other) noexcept = delete;
        ~quaternion();
        int initialize() noexcept;
        int initialize(double w, double x, double y, double z) noexcept;
        int initialize(const quaternion &other) noexcept;
        int initialize(quaternion &&other) noexcept;
        int destroy() noexcept;
        int move(quaternion &other) noexcept;
        double      get_w() const;
        double      get_x() const;
        double      get_y() const;
        double      get_z() const;
        quaternion  add(const quaternion &other) const;
        quaternion  multiply(const quaternion &other) const;
        quaternion  conjugate() const;
        double      length() const;
        quaternion  normalize() const;
        int  enable_thread_safety() noexcept;
        void disable_thread_safety() noexcept;
        bool is_thread_safe_enabled() const noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
};

#endif
