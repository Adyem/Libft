#include "game_state.hpp"
#include "game_hooks.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"
#include <new>

thread_local int ft_game_state::_last_error = FT_ERR_SUCCESS;

ft_game_state::ft_game_state() noexcept
    : _worlds(), _characters(), _variables(), _hooks(), _mutex(ft_nullptr)
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
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_game_state::~ft_game_state() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

int ft_game_state::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

void ft_game_state::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return ;
    if (this->_mutex == ft_nullptr)
        return ;
    (void)this->_mutex->unlock();
    return ;
}

ft_vector<ft_sharedptr<ft_world> > &ft_game_state::get_worlds() noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        this->set_error(lock_error);
    else
        this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_worlds);
}

ft_vector<ft_sharedptr<ft_character> > &ft_game_state::get_characters() noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        this->set_error(lock_error);
    else
        this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_characters);
}

void ft_game_state::set_variable(const ft_string &key, const ft_string &value) noexcept
{
    Pair<ft_string, ft_string> *entry;
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    entry = this->_variables.find(key);
    if (entry != this->_variables.end())
        entry->value = value;
    else
        this->_variables.insert(key, value);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_string *ft_game_state::get_variable(const ft_string &key) const noexcept
{
    const Pair<ft_string, ft_string> *entry;
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_game_state *>(this)->set_error(lock_error);
        return (ft_nullptr);
    }
    entry = this->_variables.find(key);
    if (entry == this->_variables.end())
    {
        const_cast<ft_game_state *>(this)->set_error(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return (ft_nullptr);
    }
    const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (&entry->value);
}

void ft_game_state::remove_variable(const ft_string &key) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_variables.remove(key);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void ft_game_state::clear_variables() noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_variables.clear();
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_game_state::add_character(const ft_sharedptr<ft_character> &character) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (!character)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        this->unlock_internal(lock_acquired);
        return (this->get_error());
    }
    if (character->get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(character->get_error());
        this->unlock_internal(lock_acquired);
        return (this->get_error());
    }
    this->_characters.push_back(character);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->get_error());
}

void ft_game_state::remove_character(size_t index) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    if (index >= this->_characters.size())
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->_characters.erase(this->_characters.begin() + index);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void ft_game_state::set_hooks(const ft_sharedptr<ft_game_hooks> &hooks) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_hooks = hooks;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

ft_sharedptr<ft_game_hooks> ft_game_state::get_hooks() const noexcept
{
    ft_sharedptr<ft_game_hooks> hooks_copy;
    bool lock_acquired;
    int lock_error;

    hooks_copy = ft_sharedptr<ft_game_hooks>();
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_game_state *>(this)->set_error(lock_error);
        return (hooks_copy);
    }
    hooks_copy = this->_hooks;
    const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (hooks_copy);
}

void ft_game_state::reset_hooks() noexcept
{
    ft_sharedptr<ft_game_hooks> hooks_copy;
    bool lock_acquired;
    int lock_error;

    hooks_copy = ft_sharedptr<ft_game_hooks>();
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    hooks_copy = this->_hooks;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    if (hooks_copy)
        hooks_copy->reset();
    return ;
}

void ft_game_state::dispatch_item_crafted(ft_character &character, ft_item &item) const noexcept
{
    ft_sharedptr<ft_game_hooks> hooks_copy;
    bool lock_acquired;
    int lock_error;

    hooks_copy = ft_sharedptr<ft_game_hooks>();
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_game_state *>(this)->set_error(lock_error);
        return ;
    }
    hooks_copy = this->_hooks;
    const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    if (hooks_copy)
        hooks_copy->invoke_on_item_crafted(character, item);
    return ;
}

void ft_game_state::dispatch_character_damaged(ft_character &character, int damage, uint8_t type) const noexcept
{
    ft_sharedptr<ft_game_hooks> hooks_copy;
    bool lock_acquired;
    int lock_error;

    hooks_copy = ft_sharedptr<ft_game_hooks>();
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_game_state *>(this)->set_error(lock_error);
        return ;
    }
    hooks_copy = this->_hooks;
    const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    if (hooks_copy)
        hooks_copy->invoke_on_character_damaged(character, damage, type);
    return ;
}

void ft_game_state::dispatch_event_triggered(ft_world &world, ft_event &event) const noexcept
{
    ft_sharedptr<ft_game_hooks> hooks_copy;
    bool lock_acquired;
    int lock_error;

    hooks_copy = ft_sharedptr<ft_game_hooks>();
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_game_state *>(this)->set_error(lock_error);
        return ;
    }
    hooks_copy = this->_hooks;
    const_cast<ft_game_state *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    if (hooks_copy)
        hooks_copy->invoke_on_event_triggered(world, event);
    return ;
}

int ft_game_state::get_error() const noexcept
{
    return (ft_game_state::_last_error);
}

const char *ft_game_state::get_error_str() const noexcept
{
    return (ft_strerror(ft_game_state::_last_error));
}

int ft_game_state::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_game_state::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_game_state::is_thread_safe() const noexcept
{
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

void ft_game_state::set_error(int error) const noexcept
{
    ft_game_state::_last_error = error;
    return ;
}
