#ifndef VECTOR2_HPP
# define VECTOR2_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdint>

class matrix2;

class vector2
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        double _x_component;
        double _y_component;
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialised_state = FT_CLASS_STATE_UNINITIALISED;
        static thread_local int32_t _last_error;
        static int32_t set_error(int32_t error_code) noexcept;

        friend class matrix2;
        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialised(const char *method_name) const noexcept;

        static int32_t lock_pair(const vector2 &first, const vector2 &second,
                const vector2 *&lower, const vector2 *&upper);
        static void unlock_pair(const vector2 *lower, const vector2 *upper);

    public:
        vector2() noexcept;
        vector2(double x_component, double y_component) noexcept;
        vector2(const vector2 &other) noexcept = delete;
        vector2 &operator=(const vector2 &other) = delete;
        vector2(vector2 &&other) noexcept = delete;
        vector2 &operator=(vector2 &&other) = delete;
        ~vector2() noexcept;
        int32_t initialize() noexcept;
        int32_t initialize(double x_component, double y_component) noexcept;
        int32_t initialize(const vector2 &other) noexcept;
        int32_t initialize(vector2 &&other) noexcept;
        uint32_t destroy() noexcept;
        int32_t move(vector2 &other) noexcept;
        double  get_x() const;
        double  get_y() const;
        vector2 add(const vector2 &other) const;
        vector2 subtract(const vector2 &other) const;
        double  dot(const vector2 &other) const;
        double  length() const;
        vector2 normalize() const;
        uint32_t enable_thread_safety() noexcept;
        uint32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
