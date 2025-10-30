#ifndef NULLPTR_HPP
#define NULLPTR_HPP

#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"

namespace ft
{
    class nullptr_t
    {
        private:
            mutable int _error_code;
            mutable pt_mutex _mutex;

            class mutex_guard
            {
                private:
                    pt_mutex *_mutex;
                    bool _owns_lock;
                    int _error_code;

                public:
                    mutex_guard() noexcept;
                    ~mutex_guard() noexcept;

                    mutex_guard(const mutex_guard &other) = delete;
                    mutex_guard &operator=(const mutex_guard &other) = delete;
                    mutex_guard(mutex_guard &&other) = delete;
                    mutex_guard &operator=(mutex_guard &&other) = delete;

                    int lock(pt_mutex &mutex) noexcept;
                    void unlock() noexcept;
                    bool owns_lock() const noexcept;
                    int get_error() const noexcept;
            };

            void set_error_unlocked(int error_code) const noexcept;
            void set_error(int error_code) const noexcept;
            static void restore_errno(mutex_guard &guard, int entry_errno) noexcept;

            void operator&() const;

        public:
            nullptr_t() noexcept;
            nullptr_t(const nullptr_t &other) noexcept;
            nullptr_t &operator=(const nullptr_t &other) noexcept;
            ~nullptr_t() noexcept;

            template <typename PointerType>
            operator PointerType*() const noexcept;

            template <typename ClassType, typename MemberType>
            operator MemberType ClassType::*() const noexcept;

            int get_error() const noexcept;
            const char *get_error_str() const noexcept;
    };

    extern const nullptr_t ft_nullptr_instance;
}

#define ft_nullptr (ft::ft_nullptr_instance)

template <typename PointerType>
inline ft::nullptr_t::operator PointerType*() const noexcept
{
    mutex_guard guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = guard.lock(this->_mutex);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft::nullptr_t::restore_errno(guard, entry_errno);
        return (static_cast<PointerType*>(nullptr));
    }
    this->set_error_unlocked(ER_SUCCESS);
    ft::nullptr_t::restore_errno(guard, entry_errno);
    return (static_cast<PointerType*>(nullptr));
}

template <typename ClassType, typename MemberType>
inline ft::nullptr_t::operator MemberType ClassType::*() const noexcept
{
    mutex_guard guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = guard.lock(this->_mutex);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft::nullptr_t::restore_errno(guard, entry_errno);
        return (static_cast<MemberType ClassType::*>(nullptr));
    }
    this->set_error_unlocked(ER_SUCCESS);
    ft::nullptr_t::restore_errno(guard, entry_errno);
    return (static_cast<MemberType ClassType::*>(nullptr));
}

#endif
