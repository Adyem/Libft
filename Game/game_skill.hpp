#ifndef GAME_SKILL_HPP
# define GAME_SKILL_HPP

#include "../Errno/errno.hpp"

class ft_skill
{
    private:
        int _id;
        int _level;
        int _cooldown;
        int _modifier1;
        int _modifier2;
        int _modifier3;
        int _modifier4;
        mutable int _error;

        void set_error(int err) const noexcept;

    public:
        ft_skill() noexcept;
        virtual ~ft_skill() noexcept;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        int get_level() const noexcept;
        void set_level(int level) noexcept;

        int get_cooldown() const noexcept;
        void set_cooldown(int cooldown) noexcept;
        void add_cooldown(int cooldown) noexcept;
        void sub_cooldown(int cooldown) noexcept;

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
