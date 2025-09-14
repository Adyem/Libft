#ifndef WORLD_HPP
# define WORLD_HPP

#include "event_scheduler.hpp"
#include "../Errno/errno.hpp"
#include "pathfinding.hpp"
#include "../CPP_class/class_string_class.hpp"

class ft_character;
class ft_inventory;

class ft_world
{
    private:
        ft_event_scheduler _event_scheduler;
        mutable int        _error;

        void set_error(int err) const noexcept;

    public:
        ft_world() noexcept;
        virtual ~ft_world() = default;

        void schedule_event(const ft_event &event) noexcept;
        void update_events(int ticks, const char *log_file_path = ft_nullptr, ft_string *log_buffer = ft_nullptr) noexcept;

        ft_event_scheduler       &get_event_scheduler() noexcept;
        const ft_event_scheduler &get_event_scheduler() const noexcept;

        int save_to_file(const char *file_path, const ft_character &character, const ft_inventory &inventory) const noexcept;
        int load_from_file(const char *file_path, ft_character &character, ft_inventory &inventory) noexcept;

        int plan_route(const ft_map3d &grid,
            size_t start_x, size_t start_y, size_t start_z,
            size_t goal_x, size_t goal_y, size_t goal_z,
            ft_vector<ft_path_step> &path) const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
