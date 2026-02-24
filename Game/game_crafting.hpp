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
    private:
        int             _item_id;
        int             _count;
        int             _rarity;
        mutable pt_recursive_mutex *_mutex;

        int  lock_internal(bool *lock_acquired) const noexcept;
        void unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_crafting_ingredient() noexcept;
        virtual ~ft_crafting_ingredient() noexcept;
        ft_crafting_ingredient(const ft_crafting_ingredient &other) noexcept = delete;
        ft_crafting_ingredient &operator=(const ft_crafting_ingredient &other) noexcept = delete;
        ft_crafting_ingredient(ft_crafting_ingredient &&other) noexcept = delete;
        ft_crafting_ingredient &operator=(ft_crafting_ingredient &&other) noexcept = delete;

        int initialize(const ft_crafting_ingredient &other) noexcept;
        int initialize(ft_crafting_ingredient &&other) noexcept;
        int initialize(int item_id, int count, int rarity) noexcept;

        int  enable_thread_safety() noexcept;
        int  disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;

        int get_item_id() const noexcept;
        void set_item_id(int item_id) noexcept;

        int get_count() const noexcept;
        void set_count(int count) noexcept;

        int get_rarity() const noexcept;
        void set_rarity(int rarity) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

class ft_crafting
{
    private:
        ft_map<int, ft_vector<ft_crafting_ingredient>> _recipes;
        mutable pt_recursive_mutex *_mutex;

        int  lock_internal(bool *lock_acquired) const noexcept;
        void unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_crafting() noexcept;
        virtual ~ft_crafting() noexcept;
        ft_crafting(const ft_crafting &other) noexcept = delete;
        ft_crafting &operator=(const ft_crafting &other) noexcept = delete;
        ft_crafting(ft_crafting &&other) noexcept = delete;
        ft_crafting &operator=(ft_crafting &&other) noexcept = delete;

        int  enable_thread_safety() noexcept;
        int  disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;

        ft_map<int, ft_vector<ft_crafting_ingredient>>       &get_recipes() noexcept;
        const ft_map<int, ft_vector<ft_crafting_ingredient>> &get_recipes() const noexcept;

        int  get_error() const noexcept;
        const char *get_error_str() const noexcept;

        int register_recipe(int recipe_id, ft_vector<ft_crafting_ingredient> &&ingredients) noexcept;
        int craft_item(ft_inventory &inventory, int recipe_id, const ft_sharedptr<ft_item> &result) noexcept;
};

#endif
