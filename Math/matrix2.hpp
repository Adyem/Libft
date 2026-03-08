#ifndef MATRIX2_HPP
# define MATRIX2_HPP

#include "vector2.hpp"

class matrix2
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        double _m[2][2];
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialised_state = FT_CLASS_STATE_UNINITIALISED;
        static thread_local uint32_t _last_error;
        static uint32_t set_error(uint32_t error_code) noexcept;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialised(const char *method_name) const noexcept;

        static int32_t lock_pair(const matrix2 &first, const matrix2 &second,
                const matrix2 *&lower, const matrix2 *&upper);
        static void unlock_pair(const matrix2 *lower, const matrix2 *upper);

    public:
        matrix2() noexcept;
        matrix2(double m00, double m01,
                double m10, double m11) noexcept;
        matrix2(const matrix2 &other) noexcept;
        matrix2 &operator=(const matrix2 &other) = delete;
        matrix2(matrix2 &&other) noexcept;
        matrix2 &operator=(matrix2 &&other) = delete;
        ~matrix2() noexcept;
        uint32_t initialize() noexcept;
        uint32_t initialize(double m00, double m01, double m10, double m11) noexcept;
        uint32_t initialize(const matrix2 &other) noexcept;
        uint32_t initialize(matrix2 &&other) noexcept;
        uint32_t destroy() noexcept;
        uint32_t move(matrix2 &other) noexcept;
        vector2 transform(const vector2 &vector) const;
        matrix2 multiply(const matrix2 &other) const;
        matrix2 invert() const;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
        uint32_t enable_thread_safety() noexcept;
        uint32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        uint32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
