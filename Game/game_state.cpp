#include "game_state.hpp"
#include "game_hooks.hpp"
#include "../Errno/errno.hpp"
#include <utility>
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_state_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_state_restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int ft_game_state::lock_pair(const ft_game_state &first, const ft_game_state &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_game_state *ordered_first;
    const ft_game_state *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ER_SUCCESSS;
        return (FT_ER_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_game_state *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ER_SUCCESSS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ft_errno = FT_ER_SUCCESSS;
            return (FT_ER_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_state_sleep_backoff();
    }
}

ft_game_state::ft_game_state() noexcept
    : _worlds(), _characters(), _variables(), _hooks(), _error_code(FT_ER_SUCCESSS), _mutex()
{
    ft_sharedptr<ft_world> world(new (std::nothrow) ft_world());

    if (!world)
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
    else if (world.get_error() != FT_ER_SUCCESSS)
        this->set_error(world.get_error());
    else if (world->get_error() != FT_ER_SUCCESSS)
        this->set_error(world->get_error());
    else
    {
        this->_worlds.push_back(world);
        if (this->_worlds.get_error() != FT_ER_SUCCESSS)
            this->set_error(this->_worlds.get_error());
    }
    this->set_error(this->_error_code);
    return ;
}

ft_game_state::~ft_game_state() noexcept
{
    return ;
}

ft_game_state::ft_game_state(const ft_game_state &other) noexcept
    : _worlds(), _characters(), _variables(), _hooks(), _error_code(other._error_code), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->_worlds = ft_vector<ft_sharedptr<ft_world> >();
        this->_characters = ft_vector<ft_sharedptr<ft_character> >();
        this->_error_code = other_guard.get_error();
        this->set_error(other_guard.get_error());
        game_state_restore_errno(other_guard, entry_errno);
        return ;
    }
    size_t world_index = 0;
    size_t world_count = other._worlds.size();
    while (world_index < world_count)
    {
        ft_sharedptr<ft_world> temp_world = other._worlds[world_index];
        if (temp_world.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(temp_world.get_error());
            game_state_restore_errno(other_guard, entry_errno);
            return ;
        }
        if (temp_world && temp_world->get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(temp_world->get_error());
            game_state_restore_errno(other_guard, entry_errno);
            return ;
        }
        this->_worlds.push_back(temp_world);
        if (this->_worlds.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(this->_worlds.get_error());
            game_state_restore_errno(other_guard, entry_errno);
            return ;
        }
        world_index++;
    }
    size_t character_index = 0;
    size_t character_count = other._characters.size();
    while (character_index < character_count)
    {
        ft_sharedptr<ft_character> temp = other._characters[character_index];
        if (temp.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(temp.get_error());
            game_state_restore_errno(other_guard, entry_errno);
            return ;
        }
        if (temp && temp->get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(temp->get_error());
            game_state_restore_errno(other_guard, entry_errno);
            return ;
        }
        this->_characters.push_back(temp);
        if (this->_characters.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(this->_characters.get_error());
            game_state_restore_errno(other_guard, entry_errno);
            return ;
        }
        character_index++;
    }
    this->_variables = other._variables;
    if (this->_variables.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_variables.get_error());
        game_state_restore_errno(other_guard, entry_errno);
        return ;
    }
    if (other._hooks.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other._hooks.get_error());
        game_state_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_hooks = other._hooks;
    if (this->_hooks.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_hooks.get_error());
        game_state_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->set_error(this->_error_code);
    game_state_restore_errno(other_guard, entry_errno);
    return ;
}

ft_game_state &ft_game_state::operator=(const ft_game_state &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_game_state::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_worlds.clear();
    size_t world_index = 0;
    size_t world_count = other._worlds.size();
    while (world_index < world_count)
    {
        ft_sharedptr<ft_world> temp_world = other._worlds[world_index];
        if (temp_world.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(temp_world.get_error());
            game_state_restore_errno(this_guard, entry_errno);
            game_state_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        if (temp_world && temp_world->get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(temp_world->get_error());
            game_state_restore_errno(this_guard, entry_errno);
            game_state_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        this->_worlds.push_back(temp_world);
        if (this->_worlds.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(this->_worlds.get_error());
            game_state_restore_errno(this_guard, entry_errno);
            game_state_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        world_index++;
    }
    this->_characters.clear();
    size_t character_index = 0;
    size_t character_count = other._characters.size();
    while (character_index < character_count)
    {
        ft_sharedptr<ft_character> temp = other._characters[character_index];
        if (temp.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(temp.get_error());
            game_state_restore_errno(this_guard, entry_errno);
            game_state_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        if (temp && temp->get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(temp->get_error());
            game_state_restore_errno(this_guard, entry_errno);
            game_state_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        this->_characters.push_back(temp);
        if (this->_characters.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(this->_characters.get_error());
            game_state_restore_errno(this_guard, entry_errno);
            game_state_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        character_index++;
    }
    this->_variables = other._variables;
    if (this->_variables.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_variables.get_error());
        game_state_restore_errno(this_guard, entry_errno);
        game_state_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    if (other._hooks.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other._hooks.get_error());
        game_state_restore_errno(this_guard, entry_errno);
        game_state_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_hooks = other._hooks;
    if (this->_hooks.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_hooks.get_error());
        game_state_restore_errno(this_guard, entry_errno);
        game_state_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    game_state_restore_errno(this_guard, entry_errno);
    game_state_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_game_state::ft_game_state(ft_game_state &&other) noexcept
    : _worlds(),
    _characters(),
    _variables(),
    _hooks(),
    _error_code(FT_ER_SUCCESSS),
    _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->_worlds = ft_vector<ft_sharedptr<ft_world> >();
        this->_characters = ft_vector<ft_sharedptr<ft_character> >();
        this->_error_code = other_guard.get_error();
        this->set_error(other_guard.get_error());
        game_state_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_worlds = ft_move(other._worlds);
    this->_characters = ft_move(other._characters);
    this->_variables = ft_move(other._variables);
    if (this->_variables.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_variables.get_error());
        game_state_restore_errno(other_guard, entry_errno);
        return ;
    }
    if (other._hooks.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other._hooks.get_error());
        game_state_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_hooks = ft_move(other._hooks);
    if (this->_hooks.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_hooks.get_error());
        game_state_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_error_code = other._error_code;
    size_t world_index = 0;
    size_t world_count = this->_worlds.size();
    while (world_index < world_count)
    {
        ft_sharedptr<ft_world> temp_world = this->_worlds[world_index];
        if (temp_world.get_error() != FT_ER_SUCCESSS)
            this->set_error(temp_world.get_error());
        else if (temp_world && temp_world->get_error() != FT_ER_SUCCESSS)
            this->set_error(temp_world->get_error());
        world_index++;
    }
    if (this->_worlds.get_error() != FT_ER_SUCCESSS)
        this->set_error(this->_worlds.get_error());
    if (this->_characters.get_error() != FT_ER_SUCCESSS)
        this->set_error(this->_characters.get_error());
    other._error_code = FT_ER_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ER_SUCCESSS);
    game_state_restore_errno(other_guard, entry_errno);
    return ;
}

ft_game_state &ft_game_state::operator=(ft_game_state &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_game_state::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_worlds = ft_move(other._worlds);
    this->_characters = ft_move(other._characters);
    this->_variables = ft_move(other._variables);
    if (this->_variables.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_variables.get_error());
        game_state_restore_errno(this_guard, entry_errno);
        game_state_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    if (other._hooks.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other._hooks.get_error());
        game_state_restore_errno(this_guard, entry_errno);
        game_state_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_hooks = ft_move(other._hooks);
    if (this->_hooks.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_hooks.get_error());
        game_state_restore_errno(this_guard, entry_errno);
        game_state_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_error_code = other._error_code;
    size_t world_index = 0;
    size_t world_count = this->_worlds.size();
    while (world_index < world_count)
    {
        ft_sharedptr<ft_world> temp_world = this->_worlds[world_index];
        if (temp_world.get_error() != FT_ER_SUCCESSS)
            this->set_error(temp_world.get_error());
        else if (temp_world && temp_world->get_error() != FT_ER_SUCCESSS)
            this->set_error(temp_world->get_error());
        world_index++;
    }
    if (this->_worlds.get_error() != FT_ER_SUCCESSS)
        this->set_error(this->_worlds.get_error());
    if (this->_characters.get_error() != FT_ER_SUCCESSS)
        this->set_error(this->_characters.get_error());
    other._error_code = FT_ER_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ER_SUCCESSS);
    game_state_restore_errno(this_guard, entry_errno);
    game_state_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_vector<ft_sharedptr<ft_world> > &ft_game_state::get_worlds() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return (this->_worlds);
    }
    game_state_restore_errno(guard, entry_errno);
    return (this->_worlds);
}

ft_vector<ft_sharedptr<ft_character> > &ft_game_state::get_characters() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return (this->_characters);
    }
    game_state_restore_errno(guard, entry_errno);
    return (this->_characters);
}

void ft_game_state::set_variable(const ft_string &key, const ft_string &value) noexcept
{
    int entry_errno;
    Pair<ft_string, ft_string> *entry;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    entry = this->_variables.find(key);
    if (this->_variables.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_variables.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    if (entry != this->_variables.end())
        entry->value = value;
    else
    {
        this->_variables.insert(key, value);
        if (this->_variables.get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(this->_variables.get_error());
            game_state_restore_errno(guard, entry_errno);
            return ;
        }
    }
    this->set_error(FT_ER_SUCCESSS);
    game_state_restore_errno(guard, entry_errno);
    return ;
}

const ft_string *ft_game_state::get_variable(const ft_string &key) const noexcept
{
    int entry_errno;
    const Pair<ft_string, ft_string> *entry;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_game_state *>(this)->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    entry = this->_variables.find(key);
    if (this->_variables.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_game_state *>(this)->set_error(this->_variables.get_error());
        game_state_restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (entry == this->_variables.end())
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_NOT_FOUND);
        game_state_restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    const_cast<ft_game_state *>(this)->set_error(FT_ER_SUCCESSS);
    game_state_restore_errno(guard, entry_errno);
    return (&entry->value);
}

void ft_game_state::remove_variable(const ft_string &key) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    this->_variables.remove(key);
    if (this->_variables.get_error() != FT_ER_SUCCESSS)
        this->set_error(this->_variables.get_error());
    else
        this->set_error(FT_ER_SUCCESSS);
    game_state_restore_errno(guard, entry_errno);
    return ;
}

void ft_game_state::clear_variables() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    this->_variables.clear();
    if (this->_variables.get_error() != FT_ER_SUCCESSS)
        this->set_error(this->_variables.get_error());
    else
        this->set_error(FT_ER_SUCCESSS);
    game_state_restore_errno(guard, entry_errno);
    return ;
}

int ft_game_state::add_character(const ft_sharedptr<ft_character> &character) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->set_error(FT_ER_SUCCESSS);
    if (!character)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        game_state_restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (character.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(character.get_error());
        game_state_restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (character->get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(character->get_error());
        game_state_restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    this->_characters.push_back(character);
    if (this->_characters.get_error() != FT_ER_SUCCESSS)
        this->set_error(this->_characters.get_error());
    else
        this->set_error(FT_ER_SUCCESSS);
    game_state_restore_errno(guard, entry_errno);
    return (this->_error_code);
}

void ft_game_state::remove_character(size_t index) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    if (index >= this->_characters.size())
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    this->_characters.erase(this->_characters.begin() + index);
    if (this->_characters.get_error() != FT_ER_SUCCESSS)
        this->set_error(this->_characters.get_error());
    else
        this->set_error(FT_ER_SUCCESSS);
    game_state_restore_errno(guard, entry_errno);
    return ;
}

void ft_game_state::set_hooks(const ft_sharedptr<ft_game_hooks> &hooks) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    if (hooks.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(hooks.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    this->_hooks = hooks;
    if (this->_hooks.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(this->_hooks.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(FT_ER_SUCCESSS);
    game_state_restore_errno(guard, entry_errno);
    return ;
}

ft_sharedptr<ft_game_hooks> ft_game_state::get_hooks() const noexcept
{
    int entry_errno;
    ft_sharedptr<ft_game_hooks> hooks_copy;

    entry_errno = ft_errno;
    hooks_copy = ft_sharedptr<ft_game_hooks>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_game_state *>(this)->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return (hooks_copy);
    }
    hooks_copy = this->_hooks;
    if (hooks_copy.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_game_state *>(this)->set_error(hooks_copy.get_error());
        game_state_restore_errno(guard, entry_errno);
        return (hooks_copy);
    }
    const_cast<ft_game_state *>(this)->set_error(FT_ER_SUCCESSS);
    game_state_restore_errno(guard, entry_errno);
    return (hooks_copy);
}

void ft_game_state::reset_hooks() noexcept
{
    int entry_errno;
    ft_sharedptr<ft_game_hooks> hooks_copy;

    entry_errno = ft_errno;
    hooks_copy = ft_sharedptr<ft_game_hooks>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    hooks_copy = this->_hooks;
    if (hooks_copy.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(hooks_copy.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(FT_ER_SUCCESSS);
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    if (hooks_copy)
        hooks_copy->reset();
    return ;
}

void ft_game_state::dispatch_item_crafted(ft_character &character, ft_item &item) const noexcept
{
    int entry_errno;
    ft_sharedptr<ft_game_hooks> hooks_copy;

    entry_errno = ft_errno;
    hooks_copy = ft_sharedptr<ft_game_hooks>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_game_state *>(this)->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    hooks_copy = this->_hooks;
    if (hooks_copy.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_game_state *>(this)->set_error(hooks_copy.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    const_cast<ft_game_state *>(this)->set_error(FT_ER_SUCCESSS);
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    if (hooks_copy)
        hooks_copy->invoke_on_item_crafted(character, item);
    return ;
}

void ft_game_state::dispatch_character_damaged(ft_character &character, int damage, uint8_t type) const noexcept
{
    int entry_errno;
    ft_sharedptr<ft_game_hooks> hooks_copy;

    entry_errno = ft_errno;
    hooks_copy = ft_sharedptr<ft_game_hooks>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_game_state *>(this)->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    hooks_copy = this->_hooks;
    if (hooks_copy.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_game_state *>(this)->set_error(hooks_copy.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    const_cast<ft_game_state *>(this)->set_error(FT_ER_SUCCESSS);
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    if (hooks_copy)
        hooks_copy->invoke_on_character_damaged(character, damage, type);
    return ;
}

void ft_game_state::dispatch_event_triggered(ft_world &world, ft_event &event) const noexcept
{
    int entry_errno;
    ft_sharedptr<ft_game_hooks> hooks_copy;

    entry_errno = ft_errno;
    hooks_copy = ft_sharedptr<ft_game_hooks>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_game_state *>(this)->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    hooks_copy = this->_hooks;
    if (hooks_copy.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_game_state *>(this)->set_error(hooks_copy.get_error());
        game_state_restore_errno(guard, entry_errno);
        return ;
    }
    const_cast<ft_game_state *>(this)->set_error(FT_ER_SUCCESSS);
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    if (hooks_copy)
        hooks_copy->invoke_on_event_triggered(world, event);
    return ;
}

int ft_game_state::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_game_state *>(this)->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_game_state *>(this)->set_error(error_code);
    game_state_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_game_state::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        const_cast<ft_game_state *>(this)->set_error(guard.get_error());
        game_state_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_game_state *>(this)->set_error(error_code);
    game_state_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

void ft_game_state::set_error(int error) const noexcept
{
    ft_errno = error;
    this->_error_code = error;
    return ;
}
