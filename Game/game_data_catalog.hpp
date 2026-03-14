#ifndef GAME_DATA_CATALOG_HPP
# define GAME_DATA_CATALOG_HPP

#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include "game_crafting.hpp"

class ft_item_definition
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t             _item_id;
        int32_t             _rarity;
        int32_t             _max_stack;
        int32_t             _width;
        int32_t             _height;
        int32_t             _weight;
        int32_t             _slot_requirement;
        mutable pt_recursive_mutex *_mutex;
        static thread_local int32_t     _last_error;
        uint8_t         _initialised_state;
        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        void unlock_internal(ft_bool lock_acquired) const noexcept;
        static int32_t lock_pair(const ft_item_definition &first, const ft_item_definition &second,
                ft_bool *first_locked,
                ft_bool *second_locked);
        static int32_t set_error(int32_t error_code) noexcept;

    public:
        ft_item_definition() noexcept;
        ft_item_definition(const ft_item_definition &other) noexcept;
        ft_item_definition(ft_item_definition &&other) noexcept;
        virtual ~ft_item_definition() noexcept;
        ft_item_definition &operator=(const ft_item_definition &other) noexcept = delete;
        ft_item_definition &operator=(ft_item_definition &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t initialize(int32_t item_id, int32_t rarity, int32_t max_stack, int32_t width,
            int32_t height, int32_t weight, int32_t slot_requirement) noexcept;
        int32_t initialize(const ft_item_definition &other) noexcept;
        int32_t initialize(ft_item_definition &&other) noexcept;
        int32_t move(ft_item_definition &other) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;

        int32_t get_item_id() const noexcept;
        void set_item_id(int32_t item_id) noexcept;

        int32_t get_rarity() const noexcept;
        void set_rarity(int32_t rarity) noexcept;

        int32_t get_max_stack() const noexcept;
        void set_max_stack(int32_t max_stack) noexcept;

        int32_t get_width() const noexcept;
        void set_width(int32_t width) noexcept;

        int32_t get_height() const noexcept;
        void set_height(int32_t height) noexcept;

        int32_t get_weight() const noexcept;
        void set_weight(int32_t weight) noexcept;

        int32_t get_slot_requirement() const noexcept;
        void set_slot_requirement(int32_t slot_requirement) noexcept;

};

class ft_recipe_blueprint
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t                                  _recipe_id;
        int32_t                                  _result_item_id;
        ft_vector<ft_crafting_ingredient>    _ingredients;
        mutable pt_recursive_mutex                    *_mutex;
        static thread_local int32_t                       _last_error;
        uint8_t                              _initialised_state;
        static int32_t set_error(int32_t error_code) noexcept;
        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        void unlock_internal(ft_bool lock_acquired) const noexcept;
        static int32_t lock_pair(const ft_recipe_blueprint &first, const ft_recipe_blueprint &second,
                ft_bool *first_locked,
                ft_bool *second_locked);

    public:
        ft_recipe_blueprint() noexcept;
        ft_recipe_blueprint(const ft_recipe_blueprint &other) noexcept;
        ft_recipe_blueprint(ft_recipe_blueprint &&other) noexcept;
        virtual ~ft_recipe_blueprint() noexcept;
        ft_recipe_blueprint &operator=(const ft_recipe_blueprint &other) noexcept = delete;
        ft_recipe_blueprint &operator=(ft_recipe_blueprint &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t initialize(int32_t recipe_id, int32_t result_item_id,
            const ft_vector<ft_crafting_ingredient> &ingredients) noexcept;
        int32_t initialize(const ft_recipe_blueprint &other) noexcept;
        int32_t initialize(ft_recipe_blueprint &&other) noexcept;
        int32_t move(ft_recipe_blueprint &other) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;

        int32_t get_recipe_id() const noexcept;
        void set_recipe_id(int32_t recipe_id) noexcept;

        int32_t get_result_item_id() const noexcept;
        void set_result_item_id(int32_t result_item_id) noexcept;

        ft_vector<ft_crafting_ingredient> &get_ingredients() noexcept;
        const ft_vector<ft_crafting_ingredient> &get_ingredients() const noexcept;
        void set_ingredients(const ft_vector<ft_crafting_ingredient> &ingredients) noexcept;

};

class ft_loadout_entry
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t             _slot;
        int32_t             _item_id;
        int32_t             _quantity;
        mutable pt_recursive_mutex *_mutex;
        uint8_t         _initialised_state;
        static thread_local int32_t _last_error;
        static int32_t set_error(int32_t error_code) noexcept;
        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        void unlock_internal(ft_bool lock_acquired) const noexcept;
        static int32_t lock_pair(const ft_loadout_entry &first, const ft_loadout_entry &second,
                ft_bool *first_locked,
                ft_bool *second_locked);

    public:
        ft_loadout_entry() noexcept;
        ft_loadout_entry(const ft_loadout_entry &other) noexcept;
        ft_loadout_entry(ft_loadout_entry &&other) noexcept;
        virtual ~ft_loadout_entry() noexcept;
        ft_loadout_entry &operator=(const ft_loadout_entry &other) noexcept = delete;
        ft_loadout_entry &operator=(ft_loadout_entry &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t initialize(int32_t slot, int32_t item_id, int32_t quantity) noexcept;
        int32_t initialize(const ft_loadout_entry &other) noexcept;
        int32_t initialize(ft_loadout_entry &&other) noexcept;
        int32_t move(ft_loadout_entry &other) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;

        int32_t get_slot() const noexcept;
        void set_slot(int32_t slot) noexcept;

        int32_t get_item_id() const noexcept;
        void set_item_id(int32_t item_id) noexcept;

        int32_t get_quantity() const noexcept;
        void set_quantity(int32_t quantity) noexcept;

};

class ft_loadout_blueprint
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t                           _loadout_id;
        ft_vector<ft_loadout_entry>   _entries;
        mutable pt_recursive_mutex             *_mutex;
        static thread_local int32_t                 _last_error;
        uint8_t                       _initialised_state;

        static int32_t set_error(int32_t error_code) noexcept;
        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        void unlock_internal(ft_bool lock_acquired) const noexcept;
        static int32_t lock_pair(const ft_loadout_blueprint &first, const ft_loadout_blueprint &second,
                ft_bool *first_locked,
                ft_bool *second_locked);

    public:
        ft_loadout_blueprint() noexcept;
        ft_loadout_blueprint(const ft_loadout_blueprint &other) noexcept;
        ft_loadout_blueprint(ft_loadout_blueprint &&other) noexcept;
        virtual ~ft_loadout_blueprint() noexcept;
        ft_loadout_blueprint &operator=(const ft_loadout_blueprint &other) noexcept = delete;
        ft_loadout_blueprint &operator=(ft_loadout_blueprint &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t initialize(int32_t loadout_id, const ft_vector<ft_loadout_entry> &entries) noexcept;
        int32_t initialize(const ft_loadout_blueprint &other) noexcept;
        int32_t initialize(ft_loadout_blueprint &&other) noexcept;
        int32_t move(ft_loadout_blueprint &other) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;

        int32_t get_loadout_id() const noexcept;
        void set_loadout_id(int32_t loadout_id) noexcept;

        ft_vector<ft_loadout_entry> &get_entries() noexcept;
        const ft_vector<ft_loadout_entry> &get_entries() const noexcept;
        void set_entries(const ft_vector<ft_loadout_entry> &entries) noexcept;

};

class ft_data_catalog
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        ft_map<int32_t, ft_item_definition>   _item_definitions;
        ft_map<int32_t, ft_recipe_blueprint>  _recipes;
        ft_map<int32_t, ft_loadout_blueprint> _loadouts;
        mutable pt_recursive_mutex                 *_mutex;
        static thread_local int32_t                   _last_error;
        uint8_t                           _initialised_state;

        static int32_t set_error(int32_t error_code) noexcept;
        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        void unlock_internal(ft_bool lock_acquired) const noexcept;
        static int32_t lock_pair(const ft_data_catalog &first, const ft_data_catalog &second,
                ft_bool *first_locked,
                ft_bool *second_locked);

    public:
        ft_data_catalog() noexcept;
        ft_data_catalog(const ft_data_catalog &other) noexcept;
        ft_data_catalog(ft_data_catalog &&other) noexcept;
        ~ft_data_catalog() noexcept;
        ft_data_catalog &operator=(const ft_data_catalog &other) noexcept = delete;
        ft_data_catalog &operator=(ft_data_catalog &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t initialize(const ft_data_catalog &other) noexcept;
        int32_t initialize(ft_data_catalog &&other) noexcept;
        int32_t move(ft_data_catalog &other) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;

        ft_map<int32_t, ft_item_definition> &get_item_definitions() noexcept;
        const ft_map<int32_t, ft_item_definition> &get_item_definitions() const noexcept;
        ft_map<int32_t, ft_recipe_blueprint> &get_recipes() noexcept;
        const ft_map<int32_t, ft_recipe_blueprint> &get_recipes() const noexcept;
        ft_map<int32_t, ft_loadout_blueprint> &get_loadouts() noexcept;
        const ft_map<int32_t, ft_loadout_blueprint> &get_loadouts() const noexcept;

        int32_t register_item_definition(const ft_item_definition &definition) noexcept;
        int32_t register_recipe(const ft_recipe_blueprint &recipe) noexcept;
        int32_t register_loadout(const ft_loadout_blueprint &loadout) noexcept;

        int32_t fetch_item_definition(int32_t item_id, ft_item_definition &definition) const noexcept;
        int32_t fetch_recipe(int32_t recipe_id, ft_recipe_blueprint &recipe) const noexcept;
        int32_t fetch_loadout(int32_t loadout_id, ft_loadout_blueprint &loadout) const noexcept;

};

#endif
