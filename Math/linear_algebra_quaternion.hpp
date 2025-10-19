#ifndef LINEAR_ALGEBRA_QUATERNION_HPP
# define LINEAR_ALGEBRA_QUATERNION_HPP

#include <mutex>

class quaternion
{
    private:
        double _w;
        double _x;
        double _y;
        double _z;
        mutable int _error_code;
        mutable std::mutex _mutex;

        void    set_error(int error_code) const;

    public:
        quaternion();
        quaternion(double w, double x, double y, double z);
        quaternion(const quaternion &other);
        quaternion &operator=(const quaternion &other);
        quaternion(quaternion &&other) noexcept;
        quaternion &operator=(quaternion &&other) noexcept;
        ~quaternion();
        double      get_w() const;
        double      get_x() const;
        double      get_y() const;
        double      get_z() const;
        quaternion  add(const quaternion &other) const;
        quaternion  multiply(const quaternion &other) const;
        quaternion  conjugate() const;
        double      length() const;
        quaternion  normalize() const;
        int         get_error() const;
        const char  *get_error_str() const;
};

#endif
