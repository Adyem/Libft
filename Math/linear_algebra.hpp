#ifndef LINEAR_ALGEBRA_HPP
# define LINEAR_ALGEBRA_HPP

class vector2
{
    private:
        double _x;
        double _y;
        mutable int _error_code;

        void    set_error(int error_code) const;

    public:
        vector2();
        vector2(double x, double y);
        ~vector2();
        double  get_x() const;
        double  get_y() const;
        double  dot(const vector2 &other) const;
        double  length() const;
        vector2 normalize() const;
        int     get_error() const;
        const char  *get_error_str() const;
};

class vector3
{
    private:
        double _x;
        double _y;
        double _z;
        mutable int _error_code;

        void    set_error(int error_code) const;

    public:
        vector3();
        vector3(double x, double y, double z);
        ~vector3();
        double  get_x() const;
        double  get_y() const;
        double  get_z() const;
        double  dot(const vector3 &other) const;
        vector3 cross(const vector3 &other) const;
        double  length() const;
        vector3 normalize() const;
        int     get_error() const;
        const char  *get_error_str() const;
};

class vector4
{
    private:
        double _x;
        double _y;
        double _z;
        double _w;
        mutable int _error_code;

        void    set_error(int error_code) const;

    public:
        vector4();
        vector4(double x, double y, double z, double w);
        ~vector4();
        double  get_x() const;
        double  get_y() const;
        double  get_z() const;
        double  get_w() const;
        double  dot(const vector4 &other) const;
        double  length() const;
        vector4 normalize() const;
        int     get_error() const;
        const char  *get_error_str() const;
};

class matrix4
{
    private:
        double _m[4][4];
        mutable int _error_code;

        void    set_error(int error_code) const;

    public:
        matrix4();
        matrix4(double m00, double m01, double m02, double m03,
                double m10, double m11, double m12, double m13,
                double m20, double m21, double m22, double m23,
                double m30, double m31, double m32, double m33);
        ~matrix4();
        vector4 transform(const vector4 &vector) const;
        matrix4 multiply(const matrix4 &other) const;
        matrix4 invert() const;
        int     get_error() const;
        const char  *get_error_str() const;
};

class quaternion
{
    private:
        double _w;
        double _x;
        double _y;
        double _z;
        mutable int _error_code;

        void    set_error(int error_code) const;

    public:
        quaternion();
        quaternion(double w, double x, double y, double z);
        ~quaternion();
        double      get_w() const;
        double      get_x() const;
        double      get_y() const;
        double      get_z() const;
        quaternion  multiply(const quaternion &other) const;
        double      length() const;
        quaternion  normalize() const;
        quaternion  invert() const;
        vector3     transform(const vector3 &vector) const;
        int         get_error() const;
        const char  *get_error_str() const;
};

#endif
