#ifndef EQUIPMENT_HPP
#define EQUIPMENT_HPP

#include "item.hpp"
#include "../Errno/errno.hpp"

enum ft_equipment_slot
{
    EQUIP_HEAD,
    EQUIP_CHEST,
    EQUIP_WEAPON
};

class ft_equipment
{
    private:
        ft_item _head;
        ft_item _chest;
        ft_item _weapon;
        bool _has_head;
        bool _has_chest;
        bool _has_weapon;
        mutable int _error;

        void set_error(int err) const noexcept;

    public:
        ft_equipment() noexcept;
        virtual ~ft_equipment() = default;

        int equip(int slot, const ft_item &item) noexcept;
        void unequip(int slot) noexcept;
        ft_item *get_item(int slot) noexcept;
        const ft_item *get_item(int slot) const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
