#ifndef MATRIX4_HPP
# define MATRIX4_HPP

#include "vector4.hpp"

class matrix4
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        double _m[4][4];
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialised_state = FT_CLASS_STATE_UNINITIALISED;
        static thread_local int32_t _last_error;
        static int32_t set_error(int32_t error_code) noexcept;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialised(const char *method_name) const noexcept;

        static int32_t lock_pair(const matrix4 &first, const matrix4 &second,
                const matrix4 *&lower, const matrix4 *&upper);
        static void unlock_pair(const matrix4 *lower, const matrix4 *upper);

    public:
        matrix4() noexcept;
        matrix4(double m00, double m01, double m02, double m03,
                double m10, double m11, double m12, double m13,
                double m20, double m21, double m22, double m23,
                double m30, double m31, double m32, double m33) noexcept;
        matrix4(const matrix4 &other) noexcept = delete;
        matrix4 &operator=(const matrix4 &other) = delete;
        matrix4(matrix4 &&other) noexcept = delete;
        matrix4 &operator=(matrix4 &&other) = delete;
        ~matrix4() noexcept;
        int32_t initialize() noexcept;
        int32_t initialize(double m00, double m01, double m02, double m03,
                double m10, double m11, double m12, double m13,
                double m20, double m21, double m22, double m23,
                double m30, double m31, double m32, double m33) noexcept;
        int32_t initialize(const matrix4 &other) noexcept;
        int32_t initialize(matrix4 &&other) noexcept;
        uint32_t destroy() noexcept;
        int32_t move(matrix4 &other) noexcept;
        static matrix4 make_translation(double translation_x, double translation_y, double translation_z);
        static matrix4 make_scale(double scale_x, double scale_y, double scale_z);
        static matrix4 make_rotation_x(double angle);
        static matrix4 make_rotation_y(double angle);
        static matrix4 make_rotation_z(double angle);
        vector4 transform(const vector4 &vector) const;
        matrix4 multiply(const matrix4 &other) const;
        matrix4 invert() const;
        uint32_t enable_thread_safety() noexcept;
        uint32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
