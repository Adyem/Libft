#ifndef WORLD_HPP
# define WORLD_HPP

#include "../Template/map.hpp"
#include "event.hpp"
#include "../Errno/errno.hpp"
#include "pathfinding.hpp"

class ft_character;
class ft_inventory;

class ft_world
{
    private:
        ft_map<int, ft_event> _events;
        mutable int           _error;

        void set_error(int err) const noexcept;

    public:
        ft_world() noexcept;
        virtual ~ft_world() = default;

        ft_map<int, ft_event>       &get_events() noexcept;
        const ft_map<int, ft_event> &get_events() const noexcept;

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
