#ifndef GAME_HOOKS_HPP
# define GAME_HOOKS_HPP

#include "game_character.hpp"
#include "game_item.hpp"
#include "game_world.hpp"
#include "game_event.hpp"
#include "../Template/function.hpp"
#include "../Template/vector.hpp"
#include "../Template/map.hpp"
#include "../CPP_class/class_string.hpp"
#include "../PThread/mutex.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <stdint.h>

struct ft_game_hook_metadata
{
    ft_string hook_identifier;
    ft_string listener_name;
    ft_string description;
    ft_string argument_contract;
};

struct ft_game_hook_context
{
    ft_world *world;
    ft_character *character;
    ft_item *item;
    ft_event *event;
    int integer_payload;
    uint8_t small_payload;
};

struct ft_game_hook_listener_entry
{
    ft_game_hook_metadata metadata;
    int priority;
    ft_function<void(ft_game_hook_context&)> callback;
};

extern const char *ft_game_hook_item_crafted_identifier;
extern const char *ft_game_hook_character_damaged_identifier;
extern const char *ft_game_hook_event_triggered_identifier;

ft_function<void(ft_game_hook_context&)> ft_game_hook_make_character_item_adapter(ft_function<void(ft_character&, ft_item&)> &&callback) noexcept;
ft_function<void(ft_game_hook_context&)> ft_game_hook_make_character_damage_adapter(ft_function<void(ft_character&, int, uint8_t)> &&callback) noexcept;
ft_function<void(ft_game_hook_context&)> ft_game_hook_make_world_event_adapter(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept;

class ft_game_hooks
{
    private:
        ft_function<void(ft_character&, ft_item&)> _legacy_item_crafted;
        ft_function<void(ft_character&, int, uint8_t)> _legacy_character_damaged;
        ft_function<void(ft_world&, ft_event&)> _legacy_event_triggered;
        ft_map<ft_string, ft_vector<ft_game_hook_listener_entry> > _listener_catalog;
        ft_vector<ft_game_hook_metadata> _catalog_metadata;
        pt_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

        void remove_listener_unlocked(const ft_string &hook_identifier,
            const ft_string &listener_name) noexcept;
        void insert_listener_unlocked(const ft_game_hook_listener_entry &entry) noexcept;
        void append_metadata_unlocked(const ft_game_hook_metadata &metadata) noexcept;

    public:
        ft_game_hooks() noexcept;
        ~ft_game_hooks() noexcept;
        ft_game_hooks(const ft_game_hooks &other) noexcept = delete;
        ft_game_hooks &operator=(const ft_game_hooks &other) noexcept = delete;
        ft_game_hooks(ft_game_hooks &&other) noexcept = delete;
        ft_game_hooks &operator=(ft_game_hooks &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const ft_game_hooks &other) noexcept;
        int initialize(ft_game_hooks &&other) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        void set_on_item_crafted(ft_function<void(ft_character&, ft_item&)> &&callback) noexcept;
        void set_on_character_damaged(ft_function<void(ft_character&, int, uint8_t)> &&callback) noexcept;
        void set_on_event_triggered(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept;

        ft_function<void(ft_character&, ft_item&)> get_on_item_crafted() const noexcept;
        ft_function<void(ft_character&, int, uint8_t)> get_on_character_damaged() const noexcept;
        ft_function<void(ft_world&, ft_event&)> get_on_event_triggered() const noexcept;

        void invoke_on_item_crafted(ft_character &character, ft_item &item) const noexcept;
        void invoke_on_character_damaged(ft_character &character, int damage, uint8_t type) const noexcept;
        void invoke_on_event_triggered(ft_world &world, ft_event &event) const noexcept;

        void register_listener(const ft_game_hook_metadata &metadata, int priority,
            ft_function<void(ft_game_hook_context&)> &&callback) noexcept;
        void unregister_listener(const ft_string &hook_identifier,
            const ft_string &listener_name) noexcept;
        ft_vector<ft_game_hook_metadata> get_catalog_metadata() const noexcept;
        ft_vector<ft_game_hook_metadata> get_catalog_metadata_for(const ft_string &hook_identifier) const noexcept;
        void invoke_hook(const ft_string &hook_identifier,
            ft_game_hook_context &context) const noexcept;

        void reset() noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
