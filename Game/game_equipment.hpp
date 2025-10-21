#ifndef GAME_EQUIPMENT_HPP
# define GAME_EQUIPMENT_HPP

#include "game_item.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

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
        mutable int           _error_code;
        mutable pt_mutex      _mutex;

        void set_error(int err) const noexcept;
        bool validate_item(const ft_sharedptr<ft_item> &item) noexcept;
        static int lock_pair(const ft_equipment &first, const ft_equipment &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_equipment() noexcept;
        virtual ~ft_equipment() = default;
        ft_equipment(const ft_equipment &other) noexcept;
        ft_equipment &operator=(const ft_equipment &other) noexcept;
        ft_equipment(ft_equipment &&other) noexcept;
        ft_equipment &operator=(ft_equipment &&other) noexcept;

        int equip(int slot, const ft_sharedptr<ft_item> &item) noexcept;
        void unequip(int slot) noexcept;
        ft_sharedptr<ft_item> get_item(int slot) noexcept;
        ft_sharedptr<ft_item> get_item(int slot) const noexcept;

        int         get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
