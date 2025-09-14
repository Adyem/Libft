#ifndef GAME_EVENT_HPP
# define GAME_EVENT_HPP

#include "../Template/function.hpp"

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

    public:
        ft_event() noexcept;
        virtual ~ft_event() noexcept;
        ft_event(const ft_event &other) noexcept;
        ft_event &operator=(const ft_event &other) noexcept;
        ft_event(ft_event &&other) noexcept;
        ft_event &operator=(ft_event &&other) noexcept;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        int get_duration() const noexcept;
        void set_duration(int duration) noexcept;
        void add_duration(int duration) noexcept;
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
};

#endif

