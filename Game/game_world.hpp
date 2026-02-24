#ifndef GAME_WORLD_HPP
# define GAME_WORLD_HPP

#include "game_event_scheduler.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "game_pathfinding.hpp"
#include "../CPP_class/class_string.hpp"
#include "game_world_replay.hpp"
#include "game_economy_table.hpp"
#include "game_crafting.hpp"
#include "game_dialogue_table.hpp"
#include "ft_world_region.hpp"
#include "game_quest.hpp"
#include "ft_vendor_profile.hpp"
#include "game_world_registry.hpp"
#include "game_upgrade.hpp"

class ft_character;
class ft_inventory;
class kv_store;
class ft_world_registry;
class ft_world_replay_session;
class ft_economy_table;
class ft_crafting;
class ft_dialogue_table;
class ft_world_region;
class ft_quest;
class ft_vendor_profile;
class ft_upgrade;

class ft_world
{
    private:
        ft_sharedptr<ft_event_scheduler> _event_scheduler;
        ft_sharedptr<ft_world_registry> _world_registry;
        ft_sharedptr<ft_world_replay_session> _replay_session;
        ft_sharedptr<ft_economy_table> _economy_table;
        ft_sharedptr<ft_crafting> _crafting;
        ft_sharedptr<ft_dialogue_table> _dialogue_table;
        ft_sharedptr<ft_world_region> _world_region;
        ft_sharedptr<ft_quest> _quest;
        ft_sharedptr<ft_vendor_profile> _vendor_profile;
        ft_sharedptr<ft_upgrade> _upgrade;
        mutable int        _error;

        void set_error(int err) const noexcept;
        bool propagate_scheduler_state_error() const noexcept;
        bool propagate_registry_state_error() const noexcept;
        bool propagate_replay_state_error() const noexcept;
        bool propagate_economy_state_error() const noexcept;
        bool propagate_crafting_state_error() const noexcept;
        bool propagate_dialogue_state_error() const noexcept;
        bool propagate_region_state_error() const noexcept;
        bool propagate_quest_state_error() const noexcept;
        bool propagate_vendor_profile_state_error() const noexcept;
        bool propagate_upgrade_state_error() const noexcept;
        json_group *build_snapshot_groups(const ft_character &character,
            const ft_inventory &inventory, int &error_code) const noexcept;
        int restore_from_groups(json_group *groups, ft_character &character,
            ft_inventory &inventory) noexcept;

    public:
        ft_world() noexcept;
        virtual ~ft_world() noexcept;
        ft_world(const ft_world &other) noexcept = delete;
        ft_world &operator=(const ft_world &other) noexcept = delete;
        ft_world(ft_world &&other) noexcept = delete;
        ft_world &operator=(ft_world &&other) noexcept = delete;

        void schedule_event(const ft_sharedptr<ft_event> &event) noexcept;
        void update_events(ft_sharedptr<ft_world> &self, int ticks, const char *log_file_path = ft_nullptr, ft_string *log_buffer = ft_nullptr) noexcept;

        ft_sharedptr<ft_event_scheduler>       &get_event_scheduler() noexcept;
        const ft_sharedptr<ft_event_scheduler> &get_event_scheduler() const noexcept;
        ft_sharedptr<ft_world_registry>       &get_world_registry() noexcept;
        const ft_sharedptr<ft_world_registry> &get_world_registry() const noexcept;
        ft_sharedptr<ft_world_replay_session>       &get_replay_session() noexcept;
        const ft_sharedptr<ft_world_replay_session> &get_replay_session() const noexcept;
        ft_sharedptr<ft_economy_table>       &get_economy_table() noexcept;
        const ft_sharedptr<ft_economy_table> &get_economy_table() const noexcept;
        ft_sharedptr<ft_crafting>       &get_crafting() noexcept;
        const ft_sharedptr<ft_crafting> &get_crafting() const noexcept;
        ft_sharedptr<ft_dialogue_table>       &get_dialogue_table() noexcept;
        const ft_sharedptr<ft_dialogue_table> &get_dialogue_table() const noexcept;
        ft_sharedptr<ft_world_region>       &get_world_region() noexcept;
        const ft_sharedptr<ft_world_region> &get_world_region() const noexcept;
        ft_sharedptr<ft_quest>       &get_quest() noexcept;
        const ft_sharedptr<ft_quest> &get_quest() const noexcept;
        ft_sharedptr<ft_vendor_profile>       &get_vendor_profile() noexcept;
        const ft_sharedptr<ft_vendor_profile> &get_vendor_profile() const noexcept;
        ft_sharedptr<ft_upgrade>       &get_upgrade() noexcept;
        const ft_sharedptr<ft_upgrade> &get_upgrade() const noexcept;

        int save_to_file(const char *file_path, const ft_character &character, const ft_inventory &inventory) const noexcept;
        int load_from_file(const char *file_path, ft_character &character, ft_inventory &inventory) noexcept;
        int save_to_store(kv_store &store, const char *slot_key, const ft_character &character, const ft_inventory &inventory) const noexcept;
        int load_from_store(kv_store &store, const char *slot_key, ft_character &character, ft_inventory &inventory) noexcept;
        int save_to_buffer(ft_string &out_buffer, const ft_character &character, const ft_inventory &inventory) const noexcept;
        int load_from_buffer(const char *buffer, ft_character &character, ft_inventory &inventory) noexcept;

        int plan_route(const ft_map3d &grid,
            size_t start_x, size_t start_y, size_t start_z,
            size_t goal_x, size_t goal_y, size_t goal_z,
            ft_vector<ft_path_step> &path) const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
