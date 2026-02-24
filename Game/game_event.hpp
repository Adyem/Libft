#ifndef GAME_EVENT_HPP
# define GAME_EVENT_HPP

#include "../Template/function.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"

class ft_world;

class ft_event
{
    private:
        int _id;
        int _duration;
        int _modifier1;
        int _modifier2;
        int _modifier3;
        int _modifier4;
        ft_function<void(ft_world&, ft_event&)> _callback;
        mutable int _error;
        mutable pt_recursive_mutex *_mutex;

        void set_error(int err) const noexcept;
        int lock_internal(bool *lock_acquired) const noexcept;
        void unlock_internal(bool lock_acquired) const noexcept;

        friend struct ft_event_compare_ptr;

    public:
        ft_event() noexcept;
        virtual ~ft_event() noexcept;
        ft_event(const ft_event &other) noexcept = delete;
        ft_event &operator=(const ft_event &other) noexcept = delete;
        ft_event(ft_event &&other) noexcept = delete;
        ft_event &operator=(ft_event &&other) noexcept = delete;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        int get_duration() const noexcept;
        void set_duration(int duration) noexcept;
        int add_duration(int duration) noexcept;
        void sub_duration(int duration) noexcept;

        int get_modifier1() const noexcept;
        void set_modifier1(int mod) noexcept;
        void add_modifier1(int mod) noexcept;
        void sub_modifier1(int mod) noexcept;

        int get_modifier2() const noexcept;
        void set_modifier2(int mod) noexcept;
        void add_modifier2(int mod) noexcept;
        void sub_modifier2(int mod) noexcept;

        int get_modifier3() const noexcept;
        void set_modifier3(int mod) noexcept;
        void add_modifier3(int mod) noexcept;
        void sub_modifier3(int mod) noexcept;

        int get_modifier4() const noexcept;
        void set_modifier4(int mod) noexcept;
        void add_modifier4(int mod) noexcept;
        void sub_modifier4(int mod) noexcept;

        const ft_function<void(ft_world&, ft_event&)> &get_callback() const noexcept;
        void set_callback(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
