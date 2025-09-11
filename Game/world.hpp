#ifndef WORLD_HPP
# define WORLD_HPP

#include "../Template/map.hpp"
#include "event.hpp"
#include "../Errno/errno.hpp"

class ft_character;

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

        int save_game(const char *file_path, const ft_character &character) const noexcept;
        int load_game(const char *file_path, ft_character &character) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
