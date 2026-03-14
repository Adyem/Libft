#ifndef GAME_EQUIPMENT_HPP
# define GAME_EQUIPMENT_HPP

#include "game_item.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <cstdint>

enum ft_equipment_slot
{
    EQUIP_HEAD,
    EQUIP_CHEST,
    EQUIP_WEAPON
};

class ft_equipment
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        ft_sharedptr<ft_item> _head;
        ft_sharedptr<ft_item> _chest;
        ft_sharedptr<ft_item> _weapon;
        mutable pt_recursive_mutex      *_mutex;
        uint8_t _initialised_state;
        static thread_local int32_t _last_error;

        static int32_t set_error(int32_t error_code) noexcept;

        ft_bool validate_item(const ft_sharedptr<ft_item> &item) const noexcept;
        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        void unlock_internal(ft_bool lock_acquired) const noexcept;

    public:
        ft_equipment() noexcept;
        virtual ~ft_equipment() noexcept;
        ft_equipment(const ft_equipment &other) noexcept;
        ft_equipment &operator=(const ft_equipment &other) noexcept = delete;
        ft_equipment(ft_equipment &&other) noexcept;
        ft_equipment &operator=(ft_equipment &&other) noexcept = delete;
        int32_t initialize() noexcept;
        int32_t destroy() noexcept;
        int32_t move(ft_equipment &other) noexcept;

        int32_t equip(int32_t slot, const ft_sharedptr<ft_item> &item) noexcept;
        void unequip(int32_t slot) noexcept;
        ft_sharedptr<ft_item> get_item(int32_t slot) noexcept;
        ft_sharedptr<ft_item> get_item(int32_t slot) const noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;

        int32_t         get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
