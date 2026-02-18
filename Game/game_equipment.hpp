#ifndef GAME_EQUIPMENT_HPP
# define GAME_EQUIPMENT_HPP

#include "game_item.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"

enum ft_equipment_slot
{
    EQUIP_HEAD,
    EQUIP_CHEST,
    EQUIP_WEAPON
};

class ft_equipment
{
    private:
        ft_sharedptr<ft_item> _head;
        ft_sharedptr<ft_item> _chest;
        ft_sharedptr<ft_item> _weapon;
        mutable pt_mutex      _mutex;

        bool validate_item(const ft_sharedptr<ft_item> &item) const noexcept;

    public:
        ft_equipment() noexcept;
        virtual ~ft_equipment() = default;
        ft_equipment(const ft_equipment &other) noexcept = delete;
        ft_equipment &operator=(const ft_equipment &other) noexcept = delete;
        ft_equipment(ft_equipment &&other) noexcept = delete;
        ft_equipment &operator=(ft_equipment &&other) noexcept = delete;

        int equip(int slot, const ft_sharedptr<ft_item> &item) noexcept;
        void unequip(int slot) noexcept;
        ft_sharedptr<ft_item> get_item(int slot) noexcept;
        ft_sharedptr<ft_item> get_item(int slot) const noexcept;

        int         get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
