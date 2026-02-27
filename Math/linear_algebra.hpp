#ifndef LINEAR_ALGEBRA_HPP
# define LINEAR_ALGEBRA_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdint>

class vector2
{
    private:
        double _x;
        double _y;
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialized_state = 0;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        friend class matrix2;
        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;


        static int lock_pair(const vector2 &first, const vector2 &second,
                const vector2 *&lower, const vector2 *&upper);
        static void unlock_pair(const vector2 *lower, const vector2 *upper);

    public:
        vector2();
        vector2(double x, double y);
        vector2(const vector2 &other) = delete;
        vector2 &operator=(const vector2 &other) = delete;
        vector2(vector2 &&other) = delete;
        vector2 &operator=(vector2 &&other) = delete;
        ~vector2();
        int initialize() noexcept;
        int initialize(double x, double y) noexcept;
        int initialize(const vector2 &other) noexcept;
        int initialize(vector2 &&other) noexcept;
        int destroy() noexcept;
        int move(vector2 &other) noexcept;
        double  get_x() const;
        double  get_y() const;
        vector2 add(const vector2 &other) const;
        vector2 subtract(const vector2 &other) const;
        double  dot(const vector2 &other) const;
        double  length() const;
        vector2 normalize() const;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
        int  enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
};

class vector3
{
    private:
        double _x;
        double _y;
        double _z;
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialized_state = 0;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        friend class matrix3;
        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;


        static int lock_pair(const vector3 &first, const vector3 &second,
                const vector3 *&lower, const vector3 *&upper);
        static void unlock_pair(const vector3 *lower, const vector3 *upper);

    public:
        vector3();
        vector3(double x, double y, double z);
        vector3(const vector3 &other) = delete;
        vector3 &operator=(const vector3 &other) = delete;
        vector3(vector3 &&other) = delete;
        vector3 &operator=(vector3 &&other) = delete;
        ~vector3();
        int initialize() noexcept;
        int initialize(double x, double y, double z) noexcept;
        int initialize(const vector3 &other) noexcept;
        int initialize(vector3 &&other) noexcept;
        int destroy() noexcept;
        int move(vector3 &other) noexcept;
        double  get_x() const;
        double  get_y() const;
        double  get_z() const;
        vector3 add(const vector3 &other) const;
        vector3 subtract(const vector3 &other) const;
        double  dot(const vector3 &other) const;
        vector3 cross(const vector3 &other) const;
        double  length() const;
        vector3 normalize() const;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
        int  enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
};

class vector4
{
    private:
        double _x;
        double _y;
        double _z;
        double _w;
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialized_state = 0;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        friend class matrix4;
        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;


        static int lock_pair(const vector4 &first, const vector4 &second,
                const vector4 *&lower, const vector4 *&upper);
        static void unlock_pair(const vector4 *lower, const vector4 *upper);

    public:
        vector4();
        vector4(double x, double y, double z, double w);
        vector4(const vector4 &other) = delete;
        vector4 &operator=(const vector4 &other) = delete;
        vector4(vector4 &&other) = delete;
        vector4 &operator=(vector4 &&other) = delete;
        ~vector4();
        int initialize() noexcept;
        int initialize(double x, double y, double z, double w) noexcept;
        int initialize(const vector4 &other) noexcept;
        int initialize(vector4 &&other) noexcept;
        int destroy() noexcept;
        int move(vector4 &other) noexcept;
        double  get_x() const;
        double  get_y() const;
        double  get_z() const;
        double  get_w() const;
        vector4 add(const vector4 &other) const;
        vector4 subtract(const vector4 &other) const;
        double  dot(const vector4 &other) const;
        double  length() const;
        vector4 normalize() const;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
        int  enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
};

class matrix2
{
    private:
        double _m[2][2];
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialized_state = 0;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;


        static int lock_pair(const matrix2 &first, const matrix2 &second,
                const matrix2 *&lower, const matrix2 *&upper);
        static void unlock_pair(const matrix2 *lower, const matrix2 *upper);

    public:
        matrix2();
        matrix2(double m00, double m01,
                double m10, double m11);
        matrix2(const matrix2 &other) = delete;
        matrix2 &operator=(const matrix2 &other) = delete;
        matrix2(matrix2 &&other) = delete;
        matrix2 &operator=(matrix2 &&other) = delete;
        ~matrix2();
        int initialize() noexcept;
        int initialize(double m00, double m01, double m10, double m11) noexcept;
        int initialize(const matrix2 &other) noexcept;
        int initialize(matrix2 &&other) noexcept;
        int destroy() noexcept;
        int move(matrix2 &other) noexcept;
        vector2 transform(const vector2 &vector) const;
        matrix2 multiply(const matrix2 &other) const;
        matrix2 invert() const;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
        int  enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
};

class matrix3
{
    private:
        double _m[3][3];
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialized_state = 0;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;


        static int lock_pair(const matrix3 &first, const matrix3 &second,
                const matrix3 *&lower, const matrix3 *&upper);
        static void unlock_pair(const matrix3 *lower, const matrix3 *upper);

    public:
        matrix3();
        matrix3(double m00, double m01, double m02,
                double m10, double m11, double m12,
                double m20, double m21, double m22);
        matrix3(const matrix3 &other) = delete;
        matrix3 &operator=(const matrix3 &other) = delete;
        matrix3(matrix3 &&other) = delete;
        matrix3 &operator=(matrix3 &&other) = delete;
        ~matrix3();
        int initialize() noexcept;
        int initialize(double m00, double m01, double m02,
                double m10, double m11, double m12,
                double m20, double m21, double m22) noexcept;
        int initialize(const matrix3 &other) noexcept;
        int initialize(matrix3 &&other) noexcept;
        int destroy() noexcept;
        int move(matrix3 &other) noexcept;
        vector3 transform(const vector3 &vector) const;
        matrix3 multiply(const matrix3 &other) const;
        matrix3 invert() const;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
        int  enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
};

class matrix4
{
    private:
        double _m[4][4];
        pt_recursive_mutex *_mutex = ft_nullptr;
        uint8_t _initialized_state = 0;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;


        static int lock_pair(const matrix4 &first, const matrix4 &second,
                const matrix4 *&lower, const matrix4 *&upper);
        static void unlock_pair(const matrix4 *lower, const matrix4 *upper);

    public:
        matrix4();
        matrix4(double m00, double m01, double m02, double m03,
                double m10, double m11, double m12, double m13,
                double m20, double m21, double m22, double m23,
                double m30, double m31, double m32, double m33);
        matrix4(const matrix4 &other) = delete;
        matrix4 &operator=(const matrix4 &other) = delete;
        matrix4(matrix4 &&other) = delete;
        matrix4 &operator=(matrix4 &&other) = delete;
        ~matrix4();
        int initialize() noexcept;
        int initialize(double m00, double m01, double m02, double m03,
                double m10, double m11, double m12, double m13,
                double m20, double m21, double m22, double m23,
                double m30, double m31, double m32, double m33) noexcept;
        int initialize(const matrix4 &other) noexcept;
        int initialize(matrix4 &&other) noexcept;
        int destroy() noexcept;
        int move(matrix4 &other) noexcept;
        static matrix4 make_translation(double x, double y, double z);
        static matrix4 make_scale(double x, double y, double z);
        static matrix4 make_rotation_x(double angle);
        static matrix4 make_rotation_y(double angle);
        static matrix4 make_rotation_z(double angle);
        vector4 transform(const vector4 &vector) const;
        matrix4 multiply(const matrix4 &other) const;
        matrix4 invert() const;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
        int  enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
};

# include "linear_algebra_quaternion.hpp"

#endif
