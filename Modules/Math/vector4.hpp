#ifndef VECTOR4_HPP
# define VECTOR4_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Basic/class_nullptr.hpp"
#include <cstdint>

class matrix4;

class vector4
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        double _x_component;
        double _y_component;
        double _z_component;
        double _w_component;
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialised_state = FT_CLASS_STATE_UNINITIALISED;
        static thread_local int32_t _last_error;
        static int32_t set_error(int32_t error_code) noexcept;

        friend class matrix4;
        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialised(const char *method_name) const noexcept;

        static int32_t lock_pair(const vector4 &first, const vector4 &second,
                const vector4 *&lower, const vector4 *&upper);
        static void unlock_pair(const vector4 *lower, const vector4 *upper);

    public:
        vector4() noexcept;
        vector4(double x_component, double y_component, double z_component, double w_component) noexcept;
        vector4(const vector4 &other) noexcept = delete;
        vector4 &operator=(const vector4 &other) = delete;
        vector4(vector4 &&other) noexcept = delete;
        vector4 &operator=(vector4 &&other) = delete;
        ~vector4() noexcept;
        int32_t initialize() noexcept;
        int32_t initialize(double x_component, double y_component, double z_component, double w_component) noexcept;
        int32_t initialize(const vector4 &other) noexcept;
        int32_t initialize(vector4 &&other) noexcept;
        uint32_t destroy() noexcept;
        int32_t move(vector4 &other) noexcept;
        double  get_x() const;
        double  get_y() const;
        double  get_z() const;
        double  get_w() const;
        vector4 add(const vector4 &other) const;
        vector4 subtract(const vector4 &other) const;
        double  dot(const vector4 &other) const;
        double  length() const;
        vector4 normalize() const;
        uint32_t enable_thread_safety() noexcept;
        uint32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
