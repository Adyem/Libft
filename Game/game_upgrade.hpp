#ifndef GAME_UPGRADE_HPP
# define GAME_UPGRADE_HPP

#include <cstdint>
#include "../Errno/errno.hpp"

class ft_upgrade
{
    private:
        int      _id;
        uint16_t _current_level;
        uint16_t _max_level;
        int      _modifier1;
        int      _modifier2;
        int      _modifier3;
        int      _modifier4;
        mutable int _error;

        void set_error(int err) const noexcept;

    public:
        ft_upgrade() noexcept;
        virtual ~ft_upgrade() = default;
        ft_upgrade(const ft_upgrade &other) noexcept;
        ft_upgrade &operator=(const ft_upgrade &other) noexcept;
        ft_upgrade(ft_upgrade &&other) noexcept;
        ft_upgrade &operator=(ft_upgrade &&other) noexcept;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        uint16_t get_current_level() const noexcept;
        void set_current_level(uint16_t level) noexcept;
        void add_level(uint16_t level) noexcept;
        void sub_level(uint16_t level) noexcept;

        uint16_t get_max_level() const noexcept;
        void set_max_level(uint16_t level) noexcept;

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
