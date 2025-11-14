#ifndef GAME_WORLD_HPP
# define GAME_WORLD_HPP

#include "game_event_scheduler.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "game_pathfinding.hpp"
#include "../CPP_class/class_string_class.hpp"

class ft_character;
class ft_inventory;
class kv_store;

class ft_world
{
    private:
        ft_sharedptr<ft_event_scheduler> _event_scheduler;
        mutable int        _error;

        void set_error(int err) const noexcept;
        bool propagate_scheduler_state_error() const noexcept;
        json_group *build_snapshot_groups(const ft_character &character,
            const ft_inventory &inventory, int &error_code) const noexcept;
        int restore_from_groups(json_group *groups, ft_character &character,
            ft_inventory &inventory) noexcept;

    public:
        ft_world() noexcept;
        virtual ~ft_world() = default;
        ft_world(const ft_world &other) noexcept;
        ft_world &operator=(const ft_world &other) noexcept;
        ft_world(ft_world &&other) noexcept;
        ft_world &operator=(ft_world &&other) noexcept;

        void schedule_event(const ft_sharedptr<ft_event> &event) noexcept;
        void update_events(ft_sharedptr<ft_world> &self, int ticks, const char *log_file_path = ft_nullptr, ft_string *log_buffer = ft_nullptr) noexcept;

        ft_sharedptr<ft_event_scheduler>       &get_event_scheduler() noexcept;
        const ft_sharedptr<ft_event_scheduler> &get_event_scheduler() const noexcept;

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
