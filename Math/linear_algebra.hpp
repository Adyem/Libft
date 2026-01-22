#ifndef LINEAR_ALGEBRA_HPP
# define LINEAR_ALGEBRA_HPP

#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/unique_lock.hpp"

class vector2
{
    private:
        double _x;
        double _y;
        mutable int _error_code;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};
        mutable pt_recursive_mutex _mutex;

        friend class matrix2;

        void    set_error_unlocked(int error_code) const;
        void    set_error(int error_code) const;
        void    record_operation_error(int error_code) const noexcept;
        int     lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const;
        static int lock_pair(const vector2 &first, const vector2 &second,
                ft_unique_lock<pt_recursive_mutex> &first_guard,
                ft_unique_lock<pt_recursive_mutex> &second_guard);

    public:
        vector2();
        vector2(double x, double y);
        vector2(const vector2 &other);
        vector2 &operator=(const vector2 &other);
        vector2(vector2 &&other);
        vector2 &operator=(vector2 &&other);
        ~vector2();
        double  get_x() const;
        double  get_y() const;
        vector2 add(const vector2 &other) const;
        vector2 subtract(const vector2 &other) const;
        double  dot(const vector2 &other) const;
        double  length() const;
        vector2 normalize() const;
        int     get_error() const;
        const char  *get_error_str() const;
        pt_recursive_mutex *get_mutex_for_validation() const;
};

class vector3
{
    private:
        double _x;
        double _y;
        double _z;
        mutable int _error_code;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};
        mutable pt_recursive_mutex _mutex;

        friend class matrix3;

        void    set_error_unlocked(int error_code) const;
        void    set_error(int error_code) const;
        void    record_operation_error(int error_code) const noexcept;
        int     lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const;
        static int lock_pair(const vector3 &first, const vector3 &second,
                ft_unique_lock<pt_recursive_mutex> &first_guard,
                ft_unique_lock<pt_recursive_mutex> &second_guard);

    public:
        vector3();
        vector3(double x, double y, double z);
        vector3(const vector3 &other);
        vector3 &operator=(const vector3 &other);
        vector3(vector3 &&other);
        vector3 &operator=(vector3 &&other);
        ~vector3();
        double  get_x() const;
        double  get_y() const;
        double  get_z() const;
        vector3 add(const vector3 &other) const;
        vector3 subtract(const vector3 &other) const;
        double  dot(const vector3 &other) const;
        vector3 cross(const vector3 &other) const;
        double  length() const;
        vector3 normalize() const;
        int     get_error() const;
        const char  *get_error_str() const;
        pt_recursive_mutex *get_mutex_for_validation() const;
};

class vector4
{
    private:
        double _x;
        double _y;
        double _z;
        double _w;
        mutable int _error_code;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};
        mutable pt_recursive_mutex _mutex;

        friend class matrix4;

        void    set_error_unlocked(int error_code) const;
        void    set_error(int error_code) const;
        void    record_operation_error(int error_code) const noexcept;
        int     lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const;
        static int lock_pair(const vector4 &first, const vector4 &second,
                ft_unique_lock<pt_recursive_mutex> &first_guard,
                ft_unique_lock<pt_recursive_mutex> &second_guard);

    public:
        vector4();
        vector4(double x, double y, double z, double w);
        vector4(const vector4 &other);
        vector4 &operator=(const vector4 &other);
        vector4(vector4 &&other);
        vector4 &operator=(vector4 &&other);
        ~vector4();
        double  get_x() const;
        double  get_y() const;
        double  get_z() const;
        double  get_w() const;
        vector4 add(const vector4 &other) const;
        vector4 subtract(const vector4 &other) const;
        double  dot(const vector4 &other) const;
        double  length() const;
        vector4 normalize() const;
        int     get_error() const;
        const char  *get_error_str() const;
        pt_recursive_mutex *get_mutex_for_validation() const;
};

class matrix2
{
    private:
        double _m[2][2];
        mutable int _error_code;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};
        mutable pt_recursive_mutex _mutex;

        void    set_error_unlocked(int error_code) const;
        void    set_error(int error_code) const;
        void    record_operation_error(int error_code) const noexcept;
        int     lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const;
        static int lock_pair(const matrix2 &first, const matrix2 &second,
                ft_unique_lock<pt_recursive_mutex> &first_guard,
                ft_unique_lock<pt_recursive_mutex> &second_guard);

    public:
        matrix2();
        matrix2(double m00, double m01,
                double m10, double m11);
        matrix2(const matrix2 &other);
        matrix2 &operator=(const matrix2 &other);
        matrix2(matrix2 &&other);
        matrix2 &operator=(matrix2 &&other);
        ~matrix2();
        vector2 transform(const vector2 &vector) const;
        matrix2 multiply(const matrix2 &other) const;
        matrix2 invert() const;
        int     get_error() const;
        const char  *get_error_str() const;
        pt_recursive_mutex *get_mutex_for_validation() const;
};

class matrix3
{
    private:
        double _m[3][3];
        mutable int _error_code;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};
        mutable pt_recursive_mutex _mutex;

        void    set_error_unlocked(int error_code) const;
        void    set_error(int error_code) const;
        void    record_operation_error(int error_code) const noexcept;
        int     lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const;
        static int lock_pair(const matrix3 &first, const matrix3 &second,
                ft_unique_lock<pt_recursive_mutex> &first_guard,
                ft_unique_lock<pt_recursive_mutex> &second_guard);

    public:
        matrix3();
        matrix3(double m00, double m01, double m02,
                double m10, double m11, double m12,
                double m20, double m21, double m22);
        matrix3(const matrix3 &other);
        matrix3 &operator=(const matrix3 &other);
        matrix3(matrix3 &&other);
        matrix3 &operator=(matrix3 &&other);
        ~matrix3();
        vector3 transform(const vector3 &vector) const;
        matrix3 multiply(const matrix3 &other) const;
        matrix3 invert() const;
        int     get_error() const;
        const char  *get_error_str() const;
        pt_recursive_mutex *get_mutex_for_validation() const;
};

class matrix4
{
    private:
        double _m[4][4];
        mutable int _error_code;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};
        mutable pt_recursive_mutex _mutex;

        void    set_error_unlocked(int error_code) const;
        void    set_error(int error_code) const;
        void    record_operation_error(int error_code) const noexcept;
        int     lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const;
        static int lock_pair(const matrix4 &first, const matrix4 &second,
                ft_unique_lock<pt_recursive_mutex> &first_guard,
                ft_unique_lock<pt_recursive_mutex> &second_guard);

    public:
        matrix4();
        matrix4(double m00, double m01, double m02, double m03,
                double m10, double m11, double m12, double m13,
                double m20, double m21, double m22, double m23,
                double m30, double m31, double m32, double m33);
        matrix4(const matrix4 &other);
        matrix4 &operator=(const matrix4 &other);
        matrix4(matrix4 &&other);
        matrix4 &operator=(matrix4 &&other);
        ~matrix4();
        static matrix4 make_translation(double x, double y, double z);
        static matrix4 make_scale(double x, double y, double z);
        static matrix4 make_rotation_x(double angle);
        static matrix4 make_rotation_y(double angle);
        static matrix4 make_rotation_z(double angle);
        vector4 transform(const vector4 &vector) const;
        matrix4 multiply(const matrix4 &other) const;
        matrix4 invert() const;
        int     get_error() const;
        const char  *get_error_str() const;
        pt_recursive_mutex *get_mutex_for_validation() const;
};

# include "linear_algebra_quaternion.hpp"

#endif
