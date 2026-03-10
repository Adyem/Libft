#ifndef MATRIX3_HPP
# define MATRIX3_HPP

#include "vector3.hpp"

class matrix3
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        double _m[3][3];
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialised_state = FT_CLASS_STATE_UNINITIALISED;
        static thread_local uint32_t _last_error;
        static uint32_t set_error(uint32_t error_code) noexcept;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialised(const char *method_name) const noexcept;

        static int32_t lock_pair(const matrix3 &first, const matrix3 &second,
                const matrix3 *&lower, const matrix3 *&upper);
        static void unlock_pair(const matrix3 *lower, const matrix3 *upper);

    public:
        matrix3() noexcept;
        matrix3(double m00, double m01, double m02,
                double m10, double m11, double m12,
                double m20, double m21, double m22) noexcept;
        matrix3(const matrix3 &other) noexcept;
        matrix3 &operator=(const matrix3 &other) = delete;
        matrix3(matrix3 &&other) noexcept;
        matrix3 &operator=(matrix3 &&other) = delete;
        ~matrix3() noexcept;
        int32_t initialize() noexcept;
        int32_t initialize(double m00, double m01, double m02,
                double m10, double m11, double m12,
                double m20, double m21, double m22) noexcept;
        int32_t initialize(const matrix3 &other) noexcept;
        int32_t initialize(matrix3 &&other) noexcept;
        uint32_t destroy() noexcept;
        uint32_t move(matrix3 &other) noexcept;
        vector3 transform(const vector3 &vector) const;
        matrix3 multiply(const matrix3 &other) const;
        matrix3 invert() const;
        uint32_t enable_thread_safety() noexcept;
        uint32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        uint32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
