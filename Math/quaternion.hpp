#ifndef LINEAR_ALGEBRA_QUATERNION_HPP
# define LINEAR_ALGEBRA_QUATERNION_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdint>

class quaternion
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        double _w_component;
        double _x_component;
        double _y_component;
        double _z_component;
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialised_state = FT_CLASS_STATE_UNINITIALISED;
        static thread_local uint32_t _last_error;
        static uint32_t set_error(uint32_t error_code) noexcept;

        static int32_t lock_pair(const quaternion &first, const quaternion &second,
                const quaternion *&lower, const quaternion *&upper);
        static void unlock_pair(const quaternion *lower, const quaternion *upper);
        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialised(const char *method_name) const noexcept;

    public:
        quaternion() noexcept;
        quaternion(double scalar_w, double x_component, double y_component, double z_component) noexcept;
        quaternion(const quaternion &other) noexcept;
        quaternion &operator=(const quaternion &other) = delete;
        quaternion(quaternion &&other) noexcept;
        quaternion &operator=(quaternion &&other) noexcept = delete;
        ~quaternion() noexcept;
        uint32_t initialize() noexcept;
        uint32_t initialize(double scalar_w, double x_component, double y_component, double z_component) noexcept;
        uint32_t initialize(const quaternion &other) noexcept;
        uint32_t initialize(quaternion &&other) noexcept;
        uint32_t destroy() noexcept;
        uint32_t move(quaternion &other) noexcept;
        double      get_w() const;
        double      get_x() const;
        double      get_y() const;
        double      get_z() const;
        quaternion  add(const quaternion &other) const;
        quaternion  multiply(const quaternion &other) const;
        quaternion  conjugate() const;
        double      length() const;
        quaternion  normalize() const;
        uint32_t enable_thread_safety() noexcept;
        uint32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        uint32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
};

#endif
