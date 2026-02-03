#ifndef LINEAR_ALGEBRA_QUATERNION_HPP
# define LINEAR_ALGEBRA_QUATERNION_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"

class quaternion
{
    private:
        double _w;
        double _x;
        double _y;
        double _z;
        mutable pt_recursive_mutex _mutex;
        mutable bool _thread_safe_enabled = false;

        static int lock_pair(const quaternion &first, const quaternion &second,
                const quaternion *&lower, const quaternion *&upper);
        static void unlock_pair(const quaternion *lower, const quaternion *upper);

    protected:
        int lock_mutex() const noexcept;
        int unlock_mutex() const noexcept;

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
        int  enable_thread_safety() noexcept;
        void disable_thread_safety() noexcept;
        bool is_thread_safe_enabled() const noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
};

#endif
