#ifndef GAME_DEBUFF_HPP
# define GAME_DEBUFF_HPP

#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_debuff
{
    private:
        int _id;
        int _duration;
        int _modifier1;
        int _modifier2;
        int _modifier3;
        int _modifier4;
        mutable int _error;
        mutable pt_mutex _mutex;

        void set_error(int err) const noexcept;
        static int lock_pair(const ft_debuff &first, const ft_debuff &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_debuff() noexcept;
        virtual ~ft_debuff() = default;
        ft_debuff(const ft_debuff &other) noexcept;
        ft_debuff &operator=(const ft_debuff &other) noexcept;
        ft_debuff(ft_debuff &&other) noexcept;
        ft_debuff &operator=(ft_debuff &&other) noexcept;

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

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
