#ifndef GAME_DATA_CATALOG_HPP
# define GAME_DATA_CATALOG_HPP

#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "game_crafting.hpp"

class ft_item_definition
{
    private:
        int             _item_id;
        int             _rarity;
        int             _max_stack;
        int             _width;
        int             _height;
        int             _weight;
        int             _slot_requirement;
        mutable pt_mutex _mutex;
        static int lock_pair(const ft_item_definition &first, const ft_item_definition &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_item_definition() noexcept;
        ft_item_definition(int item_id, int rarity, int max_stack, int width, int height, int weight, int slot_requirement) noexcept;
        virtual ~ft_item_definition() = default;
        ft_item_definition(const ft_item_definition &other) noexcept;
        ft_item_definition &operator=(const ft_item_definition &other) noexcept;
        ft_item_definition(ft_item_definition &&other) noexcept;
        ft_item_definition &operator=(ft_item_definition &&other) noexcept;

        int get_item_id() const noexcept;
        void set_item_id(int item_id) noexcept;

        int get_rarity() const noexcept;
        void set_rarity(int rarity) noexcept;

        int get_max_stack() const noexcept;
        void set_max_stack(int max_stack) noexcept;

        int get_width() const noexcept;
        void set_width(int width) noexcept;

        int get_height() const noexcept;
        void set_height(int height) noexcept;

        int get_weight() const noexcept;
        void set_weight(int weight) noexcept;

        int get_slot_requirement() const noexcept;
        void set_slot_requirement(int slot_requirement) noexcept;

};

class ft_recipe_blueprint
{
    private:
        int                                  _recipe_id;
        int                                  _result_item_id;
        ft_vector<ft_crafting_ingredient>    _ingredients;
        mutable pt_mutex                     _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_recipe_blueprint &first, const ft_recipe_blueprint &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_recipe_blueprint() noexcept;
        ft_recipe_blueprint(int recipe_id, int result_item_id, const ft_vector<ft_crafting_ingredient> &ingredients) noexcept;
        virtual ~ft_recipe_blueprint() = default;
        ft_recipe_blueprint(const ft_recipe_blueprint &other) noexcept;
        ft_recipe_blueprint &operator=(const ft_recipe_blueprint &other) noexcept;
        ft_recipe_blueprint(ft_recipe_blueprint &&other) noexcept;
        ft_recipe_blueprint &operator=(ft_recipe_blueprint &&other) noexcept;

        int get_recipe_id() const noexcept;
        void set_recipe_id(int recipe_id) noexcept;

        int get_result_item_id() const noexcept;
        void set_result_item_id(int result_item_id) noexcept;

        ft_vector<ft_crafting_ingredient> &get_ingredients() noexcept;
        const ft_vector<ft_crafting_ingredient> &get_ingredients() const noexcept;
        void set_ingredients(const ft_vector<ft_crafting_ingredient> &ingredients) noexcept;

};

class ft_loadout_entry
{
    private:
        int             _slot;
        int             _item_id;
        int             _quantity;
        mutable pt_mutex _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_loadout_entry &first, const ft_loadout_entry &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_loadout_entry() noexcept;
        ft_loadout_entry(int slot, int item_id, int quantity) noexcept;
        virtual ~ft_loadout_entry() = default;
        ft_loadout_entry(const ft_loadout_entry &other) noexcept;
        ft_loadout_entry &operator=(const ft_loadout_entry &other) noexcept;
        ft_loadout_entry(ft_loadout_entry &&other) noexcept;
        ft_loadout_entry &operator=(ft_loadout_entry &&other) noexcept;

        int get_slot() const noexcept;
        void set_slot(int slot) noexcept;

        int get_item_id() const noexcept;
        void set_item_id(int item_id) noexcept;

        int get_quantity() const noexcept;
        void set_quantity(int quantity) noexcept;

};

class ft_loadout_blueprint
{
    private:
        int                           _loadout_id;
        ft_vector<ft_loadout_entry>   _entries;
        mutable pt_mutex              _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_loadout_blueprint &first, const ft_loadout_blueprint &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_loadout_blueprint() noexcept;
        ft_loadout_blueprint(int loadout_id, const ft_vector<ft_loadout_entry> &entries) noexcept;
        virtual ~ft_loadout_blueprint() = default;
        ft_loadout_blueprint(const ft_loadout_blueprint &other) noexcept;
        ft_loadout_blueprint &operator=(const ft_loadout_blueprint &other) noexcept;
        ft_loadout_blueprint(ft_loadout_blueprint &&other) noexcept;
        ft_loadout_blueprint &operator=(ft_loadout_blueprint &&other) noexcept;

        int get_loadout_id() const noexcept;
        void set_loadout_id(int loadout_id) noexcept;

        ft_vector<ft_loadout_entry> &get_entries() noexcept;
        const ft_vector<ft_loadout_entry> &get_entries() const noexcept;
        void set_entries(const ft_vector<ft_loadout_entry> &entries) noexcept;

};

class ft_data_catalog
{
    private:
        ft_map<int, ft_item_definition>   _item_definitions;
        ft_map<int, ft_recipe_blueprint>  _recipes;
        ft_map<int, ft_loadout_blueprint> _loadouts;
        mutable pt_mutex                  _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_data_catalog &first, const ft_data_catalog &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_data_catalog() noexcept;
        ~ft_data_catalog() noexcept;
        ft_data_catalog(const ft_data_catalog &other) noexcept;
        ft_data_catalog &operator=(const ft_data_catalog &other) noexcept;
        ft_data_catalog(ft_data_catalog &&other) noexcept;
        ft_data_catalog &operator=(ft_data_catalog &&other) noexcept;

        ft_map<int, ft_item_definition> &get_item_definitions() noexcept;
        const ft_map<int, ft_item_definition> &get_item_definitions() const noexcept;
        ft_map<int, ft_recipe_blueprint> &get_recipes() noexcept;
        const ft_map<int, ft_recipe_blueprint> &get_recipes() const noexcept;
        ft_map<int, ft_loadout_blueprint> &get_loadouts() noexcept;
        const ft_map<int, ft_loadout_blueprint> &get_loadouts() const noexcept;

        int register_item_definition(const ft_item_definition &definition) noexcept;
        int register_recipe(const ft_recipe_blueprint &recipe) noexcept;
        int register_loadout(const ft_loadout_blueprint &loadout) noexcept;

        int fetch_item_definition(int item_id, ft_item_definition &definition) const noexcept;
        int fetch_recipe(int recipe_id, ft_recipe_blueprint &recipe) const noexcept;
        int fetch_loadout(int loadout_id, ft_loadout_blueprint &loadout) const noexcept;

};

#endif
