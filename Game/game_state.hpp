#ifndef GAME_STATE_HPP
# define GAME_STATE_HPP

#include "game_world.hpp"
#include "game_character.hpp"
#include "game_item.hpp"
#include "game_event.hpp"
#include "../Template/vector.hpp"
#include "../Template/map.hpp"
#include "../Template/shared_ptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_game_hooks;

class ft_game_state
{
    private:
        ft_vector<ft_sharedptr<ft_world> >     _worlds;
        ft_vector<ft_sharedptr<ft_character> > _characters;
        ft_map<ft_string, ft_string>           _variables;
        ft_sharedptr<ft_game_hooks>            _hooks;
        mutable int                            _error_code;
        mutable pt_mutex                       _mutex;

        void set_error(int error) const noexcept;
        static int lock_pair(const ft_game_state &first, const ft_game_state &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_game_state() noexcept;
        ~ft_game_state() noexcept;
        ft_game_state(const ft_game_state &other) noexcept;
        ft_game_state &operator=(const ft_game_state &other) noexcept;
        ft_game_state(ft_game_state &&other) noexcept;
        ft_game_state &operator=(ft_game_state &&other) noexcept;

        ft_vector<ft_sharedptr<ft_world> > &get_worlds() noexcept;

        ft_vector<ft_sharedptr<ft_character> > &get_characters() noexcept;

        void set_variable(const ft_string &key, const ft_string &value) noexcept;
        const ft_string *get_variable(const ft_string &key) const noexcept;
        void remove_variable(const ft_string &key) noexcept;
        void clear_variables() noexcept;

        int add_character(const ft_sharedptr<ft_character> &character) noexcept;
        void remove_character(size_t index) noexcept;

        void set_hooks(const ft_sharedptr<ft_game_hooks> &hooks) noexcept;
        ft_sharedptr<ft_game_hooks> get_hooks() const noexcept;
        void reset_hooks() noexcept;

        void dispatch_item_crafted(ft_character &character, ft_item &item) const noexcept;
        void dispatch_character_damaged(ft_character &character, int damage, uint8_t type) const noexcept;
        void dispatch_event_triggered(ft_world &world, ft_event &event) const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
