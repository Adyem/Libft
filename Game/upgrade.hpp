#ifndef UPGRADE_HPP
# define UPGRADE_HPP

#include <cstdint>

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

    public:
        ft_upgrade() noexcept;
        virtual ~ft_upgrade() = default;

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
};

#endif
