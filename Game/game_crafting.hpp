#ifndef GAME_CRAFTING_HPP
# define GAME_CRAFTING_HPP

#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "game_inventory.hpp"
#include "game_item.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"

struct ft_crafting_ingredient
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t             _item_id;
        int32_t             _count;
        int32_t             _rarity;
        uint8_t             _initialised_state;
        mutable pt_recursive_mutex *_mutex;
        static thread_local int32_t _last_error;

        static int32_t set_error(int32_t error_code) noexcept;

        int32_t  lock_internal(ft_bool *lock_acquired) const noexcept;
        void unlock_internal(ft_bool lock_acquired) const noexcept;

    public:
        ft_crafting_ingredient() noexcept;
        ft_crafting_ingredient(const ft_crafting_ingredient &other) noexcept;
        ft_crafting_ingredient(ft_crafting_ingredient &&other) noexcept;
        virtual ~ft_crafting_ingredient() noexcept;
        ft_crafting_ingredient &operator=(const ft_crafting_ingredient &other) noexcept = delete;
        ft_crafting_ingredient &operator=(ft_crafting_ingredient &&other) noexcept = delete;

        int32_t initialize(const ft_crafting_ingredient &other) noexcept;
        int32_t initialize(ft_crafting_ingredient &&other) noexcept;
        int32_t move(ft_crafting_ingredient &other) noexcept;
        int32_t destroy() noexcept;
        int32_t initialize(int32_t item_id, int32_t count, int32_t rarity) noexcept;

        int32_t  enable_thread_safety() noexcept;
        int32_t  disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;

        int32_t get_item_id() const noexcept;
        void set_item_id(int32_t item_id) noexcept;

        int32_t get_count() const noexcept;
        void set_count(int32_t count) noexcept;

        int32_t get_rarity() const noexcept;
        void set_rarity(int32_t rarity) noexcept;

        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

class ft_crafting
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        ft_map<int32_t, ft_vector<ft_crafting_ingredient>> _recipes;
        uint8_t                                     _initialised_state;
        mutable pt_recursive_mutex *_mutex;
        static thread_local int32_t _last_error;

        static int32_t set_error(int32_t error_code) noexcept;

        int32_t  lock_internal(ft_bool *lock_acquired) const noexcept;
        void unlock_internal(ft_bool lock_acquired) const noexcept;

    public:
        ft_crafting() noexcept;
        ft_crafting(const ft_crafting &other) noexcept;
        ft_crafting(ft_crafting &&other) noexcept;
        virtual ~ft_crafting() noexcept;
        ft_crafting &operator=(const ft_crafting &other) noexcept = delete;
        ft_crafting &operator=(ft_crafting &&other) noexcept = delete;

        int32_t  enable_thread_safety() noexcept;
        int32_t  disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;

        ft_map<int32_t, ft_vector<ft_crafting_ingredient>>       &get_recipes() noexcept;
        const ft_map<int32_t, ft_vector<ft_crafting_ingredient>> &get_recipes() const noexcept;

        int32_t  get_error() const noexcept;
        const char *get_error_str() const noexcept;

        int32_t register_recipe(int32_t recipe_id, ft_vector<ft_crafting_ingredient> &&ingredients) noexcept;
        int32_t craft_item(ft_inventory &inventory, int32_t recipe_id, const ft_sharedptr<ft_item> &result) noexcept;
        int32_t initialize() noexcept;
        int32_t destroy() noexcept;
        int32_t move(ft_crafting &other) noexcept;
};

#endif
