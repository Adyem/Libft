#include "../PThread/pthread_internal.hpp"
#include "game_state.hpp"
#include "game_hooks.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_game_state::_last_error = FT_ERR_SUCCESS;

ft_game_state::ft_game_state() noexcept
    : _worlds(), _characters(), _variables(), _hooks(), _mutex(ft_nullptr),
      _initialized_state(ft_game_state::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_game_state::~ft_game_state() noexcept
{
    if (this->_initialized_state == ft_game_state::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_game_state::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_game_state lifecycle error: %s: %s\n", method_name,
        reason);
    su_abort();
    return ;
}

void ft_game_state::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_game_state::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_game_state::initialize() noexcept
{
    ft_sharedptr<ft_world> world;
    int worlds_error;
    int characters_error;
    int variables_error;

    if (this->_initialized_state == ft_game_state::_state_initialized)
    {
        this->abort_lifecycle_error("ft_game_state::initialize",
            "called while object is already initialized");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    worlds_error = this->_worlds.initialize();
    if (worlds_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_game_state::_state_destroyed;
        this->set_error(worlds_error);
        return (worlds_error);
    }
    characters_error = this->_characters.initialize();
    if (characters_error != FT_ERR_SUCCESS)
    {
        (void)this->_worlds.destroy();
        this->_initialized_state = ft_game_state::_state_destroyed;
        this->set_error(characters_error);
        return (characters_error);
    }
    variables_error = this->_variables.initialize();
    if (variables_error != FT_ERR_SUCCESS)
    {
        (void)this->_worlds.destroy();
        (void)this->_characters.destroy();
        this->_initialized_state = ft_game_state::_state_destroyed;
        this->set_error(variables_error);
        return (variables_error);
    }
    world = ft_sharedptr<ft_world>(new (std::nothrow) ft_world());
    if (!world || world->get_error() != FT_ERR_SUCCESS)
    {
        (void)this->_worlds.destroy();
        (void)this->_characters.destroy();
        (void)this->_variables.destroy();
        this->_initialized_state = ft_game_state::_state_destroyed;
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    this->_worlds.push_back(world);
    this->_hooks = ft_sharedptr<ft_game_hooks>();
    this->_initialized_state = ft_game_state::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_game_state::destroy() noexcept
{
    int worlds_error;
    int characters_error;
    int variables_error;
    int disable_error;
    int final_error;

    if (this->_initialized_state != ft_game_state::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    worlds_error = this->_worlds.destroy();
    characters_error = this->_characters.destroy();
    variables_error = this->_variables.destroy();
    this->_hooks = ft_sharedptr<ft_game_hooks>();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_game_state::_state_destroyed;
    final_error = worlds_error;
    if (final_error == FT_ERR_SUCCESS && characters_error != FT_ERR_SUCCESS)
        final_error = characters_error;
    if (final_error == FT_ERR_SUCCESS && variables_error != FT_ERR_SUCCESS)
        final_error = variables_error;
    if (final_error == FT_ERR_SUCCESS && disable_error != FT_ERR_SUCCESS)
        final_error = disable_error;
    this->set_error(final_error);
    return (final_error);
}

int ft_game_state::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    this->abort_if_not_initialized("ft_game_state::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

void ft_game_state::unlock_internal(bool lock_acquired) const noexcept
{
    int unlock_error;

    this->abort_if_not_initialized("ft_game_state::unlock_internal");
    if (lock_acquired == false)
        return ;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        const_cast<ft_game_state *>(this)->set_error(unlock_error);
    return ;
}

ft_vector<ft_sharedptr<ft_world> > &ft_game_state::get_worlds() noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_game_state::get_worlds");
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

    this->abort_if_not_initialized("ft_game_state::get_characters");
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

    this->abort_if_not_initialized("ft_game_state::set_variable");
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

    this->abort_if_not_initialized("ft_game_state::get_variable");
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

    this->abort_if_not_initialized("ft_game_state::remove_variable");
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

    this->abort_if_not_initialized("ft_game_state::clear_variables");
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

    this->abort_if_not_initialized("ft_game_state::add_character");
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

    this->abort_if_not_initialized("ft_game_state::remove_character");
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

    this->abort_if_not_initialized("ft_game_state::set_hooks");
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

    this->abort_if_not_initialized("ft_game_state::get_hooks");
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

    this->abort_if_not_initialized("ft_game_state::reset_hooks");
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

    this->abort_if_not_initialized("ft_game_state::dispatch_item_crafted");
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

    this->abort_if_not_initialized("ft_game_state::dispatch_character_damaged");
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

    this->abort_if_not_initialized("ft_game_state::dispatch_event_triggered");
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

    this->abort_if_not_initialized("ft_game_state::enable_thread_safety");
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

    this->abort_if_not_initialized("ft_game_state::disable_thread_safety");
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
    this->abort_if_not_initialized("ft_game_state::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

void ft_game_state::set_error(int error) const noexcept
{
    ft_game_state::_last_error = error;
    return ;
}
