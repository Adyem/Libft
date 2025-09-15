#ifndef GAME_STATE_HPP
# define GAME_STATE_HPP

#include "game_world.hpp"
#include "game_character.hpp"
#include "../Template/vector.hpp"
#include "../Template/shared_ptr.hpp"

class ft_game_state
{
    private:
        ft_vector<ft_sharedptr<ft_world> >     _worlds;
        ft_vector<ft_sharedptr<ft_character> > _characters;
        mutable int                            _error_code;

        void set_error(int error) const noexcept;

    public:
        ft_game_state() noexcept;
        ~ft_game_state() noexcept;
        ft_game_state(const ft_game_state &other) noexcept;
        ft_game_state &operator=(const ft_game_state &other) noexcept;
        ft_game_state(ft_game_state &&other) noexcept;
        ft_game_state &operator=(ft_game_state &&other) noexcept;

        ft_vector<ft_sharedptr<ft_world> > &get_worlds() noexcept;

        ft_vector<ft_sharedptr<ft_character> > &get_characters() noexcept;

        int add_character(const ft_sharedptr<ft_character> &character) noexcept;
        void remove_character(size_t index) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
