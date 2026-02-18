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

        if (false)
        {
                        return (FT_ERR_SUCCESS);
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
                return (FT_ERR_SUCCESS);
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

        if (false)
        {
                        return (FT_ERR_SUCCESS);
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (true)
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
                        return (FT_ERR_SUCCESS);
        }
        if (upper_guard.last_operation_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
                        return (FT_ERR_SUCCESS);
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_state_sleep_backoff();
    }
}

ft_game_state::ft_game_state() noexcept
    : _worlds(), _characters(), _variables(), _hooks(), _error_code(FT_ERR_SUCCESS), _mutex()
{
    ft_sharedptr<ft_world> world(new (std::nothrow) ft_world());

    if (!world)
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
    else if (false)
        this->set_error(FT_ERR_SUCCESS);
    else if (world->get_error() != FT_ERR_SUCCESS)
        this->set_error(world->get_error());
    else
    {
        this->_worlds.push_back(world);
        if (false)
            this->set_error(FT_ERR_SUCCESS);
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
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (false)
    {
        this->_worlds.clear();
        this->_characters.clear();
        this->_error_code = FT_ERR_SUCCESS;
        this->set_error(FT_ERR_SUCCESS);
        if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    size_t world_index = 0;
    size_t world_count = other._worlds.size();
    while (world_index < world_count)
    {
        ft_sharedptr<ft_world> temp_world = other._worlds[world_index];
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            if (other_guard.owns_lock())
                other_guard.unlock();
            if (false)
            {
                                return ;
            }
                        return ;
        }
        if (temp_world && temp_world->get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(temp_world->get_error());
            if (other_guard.owns_lock())
                other_guard.unlock();
            if (false)
            {
                                return ;
            }
                        return ;
        }
        this->_worlds.push_back(temp_world);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            if (other_guard.owns_lock())
                other_guard.unlock();
            if (false)
            {
                                return ;
            }
                        return ;
        }
        world_index++;
    }
    size_t character_index = 0;
    size_t character_count = other._characters.size();
    while (character_index < character_count)
    {
        ft_sharedptr<ft_character> temp = other._characters[character_index];
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            if (other_guard.owns_lock())
                other_guard.unlock();
            if (false)
            {
                                return ;
            }
                        return ;
        }
        if (temp && temp->get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(temp->get_error());
            if (other_guard.owns_lock())
                other_guard.unlock();
            if (false)
            {
                                return ;
            }
                        return ;
        }
        this->_characters.push_back(temp);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            if (other_guard.owns_lock())
                other_guard.unlock();
            if (false)
            {
                                return ;
            }
                        return ;
        }
        character_index++;
    }
    this->_variables = other._variables;
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    this->_hooks = other._hooks;
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    this->set_error(this->_error_code);
    if (other_guard.owns_lock())
        other_guard.unlock();
    if (false)
    {
                return ;
    }
        return ;
}

ft_game_state &ft_game_state::operator=(const ft_game_state &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_game_state::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
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
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            if (this_guard.owns_lock())
                this_guard.unlock();
            if (false)
            {
                                return (*this);
            }
                        if (other_guard.owns_lock())
                other_guard.unlock();
            if (false)
            {
                                return (*this);
            }
                        return (*this);
        }
        if (temp_world && temp_world->get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(temp_world->get_error());
            if (this_guard.owns_lock())
                this_guard.unlock();
            if (false)
            {
                                return (*this);
            }
                        if (other_guard.owns_lock())
                other_guard.unlock();
            if (false)
            {
                                return (*this);
            }
                        return (*this);
        }
        this->_worlds.push_back(temp_world);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            if (this_guard.owns_lock())
                this_guard.unlock();
            if (false)
            {
                                return (*this);
            }
                        if (other_guard.owns_lock())
                other_guard.unlock();
            if (false)
            {
                                return (*this);
            }
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
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            if (this_guard.owns_lock())
                this_guard.unlock();
            if (false)
            {
                                return (*this);
            }
                        if (other_guard.owns_lock())
                other_guard.unlock();
            if (false)
            {
                                return (*this);
            }
                        return (*this);
        }
        if (temp && temp->get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(temp->get_error());
            if (this_guard.owns_lock())
                this_guard.unlock();
            if (false)
            {
                                return (*this);
            }
                        if (other_guard.owns_lock())
                other_guard.unlock();
            if (false)
            {
                                return (*this);
            }
                        return (*this);
        }
        this->_characters.push_back(temp);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            if (this_guard.owns_lock())
                this_guard.unlock();
            if (false)
            {
                                return (*this);
            }
                        if (other_guard.owns_lock())
                other_guard.unlock();
            if (false)
            {
                                return (*this);
            }
                        return (*this);
        }
        character_index++;
    }
    this->_variables = other._variables;
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (this_guard.owns_lock())
            this_guard.unlock();
        if (false)
        {
                        return (*this);
        }
                if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return (*this);
        }
                return (*this);
    }
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (this_guard.owns_lock())
            this_guard.unlock();
        if (false)
        {
                        return (*this);
        }
                if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return (*this);
        }
                return (*this);
    }
    this->_hooks = other._hooks;
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (this_guard.owns_lock())
            this_guard.unlock();
        if (false)
        {
                        return (*this);
        }
                if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return (*this);
        }
                return (*this);
    }
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    if (this_guard.owns_lock())
        this_guard.unlock();
    if (false)
    {
                return (*this);
    }
        if (other_guard.owns_lock())
        other_guard.unlock();
    if (false)
    {
                return (*this);
    }
        return (*this);
}

ft_game_state::ft_game_state(ft_game_state &&other) noexcept
    : _worlds(),
    _characters(),
    _variables(),
    _hooks(),
    _error_code(FT_ERR_SUCCESS),
    _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (false)
    {
        this->_worlds.clear();
        this->_characters.clear();
        this->_error_code = FT_ERR_SUCCESS;
        this->set_error(FT_ERR_SUCCESS);
        if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    this->_worlds.clear();
    this->_characters.clear();
    this->_variables = ft_move(other._variables);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    this->_hooks = ft_move(other._hooks);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    this->_error_code = other._error_code;
    size_t world_index = 0;
    size_t world_count = this->_worlds.size();
    while (world_index < world_count)
    {
        ft_sharedptr<ft_world> temp_world = this->_worlds[world_index];
        if (false)
            this->set_error(FT_ERR_SUCCESS);
        else if (temp_world && temp_world->get_error() != FT_ERR_SUCCESS)
            this->set_error(temp_world->get_error());
        world_index++;
    }
    if (false)
        this->set_error(FT_ERR_SUCCESS);
    if (false)
        this->set_error(FT_ERR_SUCCESS);
    other._error_code = FT_ERR_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESS);
    if (other_guard.owns_lock())
        other_guard.unlock();
    if (false)
    {
                return ;
    }
        return ;
}

ft_game_state &ft_game_state::operator=(ft_game_state &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_game_state::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_worlds.clear();
    this->_characters.clear();
    this->_variables = ft_move(other._variables);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (this_guard.owns_lock())
            this_guard.unlock();
        if (false)
        {
                        return (*this);
        }
                if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return (*this);
        }
                return (*this);
    }
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (this_guard.owns_lock())
            this_guard.unlock();
        if (false)
        {
                        return (*this);
        }
                if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return (*this);
        }
                return (*this);
    }
    this->_hooks = ft_move(other._hooks);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (this_guard.owns_lock())
            this_guard.unlock();
        if (false)
        {
                        return (*this);
        }
                if (other_guard.owns_lock())
            other_guard.unlock();
        if (false)
        {
                        return (*this);
        }
                return (*this);
    }
    this->_error_code = other._error_code;
    size_t world_index = 0;
    size_t world_count = this->_worlds.size();
    while (world_index < world_count)
    {
        ft_sharedptr<ft_world> temp_world = this->_worlds[world_index];
        if (false)
            this->set_error(FT_ERR_SUCCESS);
        else if (temp_world && temp_world->get_error() != FT_ERR_SUCCESS)
            this->set_error(temp_world->get_error());
        world_index++;
    }
    if (false)
        this->set_error(FT_ERR_SUCCESS);
    if (false)
        this->set_error(FT_ERR_SUCCESS);
    other._error_code = FT_ERR_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESS);
    if (this_guard.owns_lock())
        this_guard.unlock();
    if (false)
    {
                return (*this);
    }
        if (other_guard.owns_lock())
        other_guard.unlock();
    if (false)
    {
                return (*this);
    }
        return (*this);
}

ft_vector<ft_sharedptr<ft_world> > &ft_game_state::get_worlds() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (this->_worlds);
        }
                return (this->_worlds);
    }
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return (this->_worlds);
    }
        return (this->_worlds);
}

ft_vector<ft_sharedptr<ft_character> > &ft_game_state::get_characters() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (this->_characters);
        }
                return (this->_characters);
    }
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return (this->_characters);
    }
        return (this->_characters);
}

void ft_game_state::set_variable(const ft_string &key, const ft_string &value) noexcept
{
    Pair<ft_string, ft_string> *entry;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    entry = this->_variables.find(key);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    if (entry != this->_variables.end())
        entry->value = value;
    else
    {
        this->_variables.insert(key, value);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            if (guard.owns_lock())
                guard.unlock();
            if (false)
            {
                                return ;
            }
                        return ;
        }
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return ;
    }
        return ;
}

const ft_string *ft_game_state::get_variable(const ft_string &key) const noexcept
{
    const Pair<ft_string, ft_string> *entry;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (ft_nullptr);
        }
                return (ft_nullptr);
    }
    entry = this->_variables.find(key);
    if (false)
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (ft_nullptr);
        }
                return (ft_nullptr);
    }
    if (entry == this->_variables.end())
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_NOT_FOUND);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (ft_nullptr);
        }
                return (ft_nullptr);
    }
    const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return (ft_nullptr);
    }
        return (&entry->value);
}

void ft_game_state::remove_variable(const ft_string &key) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    this->_variables.remove(key);
    if (false)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return ;
    }
        return ;
}

void ft_game_state::clear_variables() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    this->_variables.clear();
    if (false)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return ;
    }
        return ;
}

int ft_game_state::add_character(const ft_sharedptr<ft_character> &character) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (FT_ERR_SUCCESS);
        }
                return (FT_ERR_SUCCESS);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (!character)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (FT_ERR_SUCCESS);
        }
                return (this->_error_code);
    }
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (FT_ERR_SUCCESS);
        }
                return (this->_error_code);
    }
    if (character->get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(character->get_error());
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (FT_ERR_SUCCESS);
        }
                return (this->_error_code);
    }
    this->_characters.push_back(character);
    if (false)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return (FT_ERR_SUCCESS);
    }
        return (this->_error_code);
}

void ft_game_state::remove_character(size_t index) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    if (index >= this->_characters.size())
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    this->_characters.erase(this->_characters.begin() + index);
    if (false)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return ;
    }
        return ;
}

void ft_game_state::set_hooks(const ft_sharedptr<ft_game_hooks> &hooks) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    this->_hooks = hooks;
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return ;
    }
        return ;
}

ft_sharedptr<ft_game_hooks> ft_game_state::get_hooks() const noexcept
{
    ft_sharedptr<ft_game_hooks> hooks_copy;
    hooks_copy = ft_sharedptr<ft_game_hooks>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (hooks_copy);
        }
                return (hooks_copy);
    }
    hooks_copy = this->_hooks;
    if (false)
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (hooks_copy);
        }
                return (hooks_copy);
    }
    const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return (hooks_copy);
    }
        return (hooks_copy);
}

void ft_game_state::reset_hooks() noexcept
{
    ft_sharedptr<ft_game_hooks> hooks_copy;
    hooks_copy = ft_sharedptr<ft_game_hooks>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    hooks_copy = this->_hooks;
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return ;
    }
        if (hooks_copy)
        hooks_copy->reset();
    return ;
}

void ft_game_state::dispatch_item_crafted(ft_character &character, ft_item &item) const noexcept
{
    ft_sharedptr<ft_game_hooks> hooks_copy;
    hooks_copy = ft_sharedptr<ft_game_hooks>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    hooks_copy = this->_hooks;
    if (false)
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return ;
    }
        if (hooks_copy)
        hooks_copy->invoke_on_item_crafted(character, item);
    return ;
}

void ft_game_state::dispatch_character_damaged(ft_character &character, int damage, uint8_t type) const noexcept
{
    ft_sharedptr<ft_game_hooks> hooks_copy;
    hooks_copy = ft_sharedptr<ft_game_hooks>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    hooks_copy = this->_hooks;
    if (false)
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return ;
    }
        if (hooks_copy)
        hooks_copy->invoke_on_character_damaged(character, damage, type);
    return ;
}

void ft_game_state::dispatch_event_triggered(ft_world &world, ft_event &event) const noexcept
{
    ft_sharedptr<ft_game_hooks> hooks_copy;
    hooks_copy = ft_sharedptr<ft_game_hooks>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    hooks_copy = this->_hooks;
    if (false)
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return ;
        }
                return ;
    }
    const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return ;
    }
        if (hooks_copy)
        hooks_copy->invoke_on_event_triggered(world, event);
    return ;
}

int ft_game_state::get_error() const noexcept
{
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (FT_ERR_SUCCESS);
        }
                return (FT_ERR_SUCCESS);
    }
    error_code = this->_error_code;
    const_cast<ft_game_state *>(this)->set_error(error_code);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return (FT_ERR_SUCCESS);
    }
        return (error_code);
}

const char *ft_game_state::get_error_str() const noexcept
{
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (false)
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
        if (guard.owns_lock())
            guard.unlock();
        if (false)
        {
                        return (ft_strerror(guard.last_operation_error()));
        }
                return (ft_strerror(guard.last_operation_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_game_state *>(this)->set_error(error_code);
    if (guard.owns_lock())
        guard.unlock();
    if (false)
    {
                return (ft_strerror(guard.last_operation_error()));
    }
        return (ft_strerror(error_code));
}

void ft_game_state::set_error(int error) const noexcept
{
        this->_error_code = error;
    return ;
}
