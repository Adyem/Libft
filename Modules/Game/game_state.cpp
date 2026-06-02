#include "../PThread/pthread_internal.hpp"
#include "game_state.hpp"
#include "game_hooks.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/map.hpp"
#include "../Template/pair.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Template/vector.hpp"
#include "game_achievement.hpp"
#include "game_buff.hpp"
#include "game_crafting.hpp"
#include "game_currency_rate.hpp"
#include "game_debuff.hpp"
#include "game_dialogue_line.hpp"
#include "game_dialogue_script.hpp"
#include "game_dialogue_table.hpp"
#include "game_economy_table.hpp"
#include "game_pathfinding.hpp"
#include "game_price_definition.hpp"
#include "game_quest.hpp"
#include "game_rarity_band.hpp"
#include "game_region_definition.hpp"
#include "game_skill.hpp"
#include "game_upgrade.hpp"
#include "game_vendor_profile.hpp"
#include "game_world_region.hpp"
#include "game_world_registry.hpp"
#include "game_world_replay.hpp"

thread_local int32_t game_state::_last_error = FT_ERR_SUCCESS;

game_state::game_state() noexcept
    : _worlds(), _characters(), _variables(), _hooks(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_state::~game_state() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t game_state::initialize() noexcept
{
    ft_sharedptr<game_world> world;
    int32_t worlds_error;
    int32_t characters_error;
    int32_t variables_error;
    int32_t hooks_error;
    int32_t move_error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_state::initialize",
            "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    worlds_error = this->_worlds.initialize();
    if (worlds_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(worlds_error);
        return (worlds_error);
    }
    characters_error = this->_characters.initialize();
    if (characters_error != FT_ERR_SUCCESS)
    {
        (void)this->_worlds.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(characters_error);
        return (characters_error);
    }
    variables_error = this->_variables.initialize();
    if (variables_error != FT_ERR_SUCCESS)
    {
        (void)this->_worlds.destroy();
        (void)this->_characters.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(variables_error);
        return (variables_error);
    }
    hooks_error = this->_hooks.initialize();
    if (hooks_error != FT_ERR_SUCCESS)
    {
        (void)this->_variables.destroy();
        (void)this->_worlds.destroy();
        (void)this->_characters.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(hooks_error);
        return (hooks_error);
    }
    world = ft_sharedptr<game_world>(new (std::nothrow) game_world());
    if (!world)
    {
        (void)this->_hooks.destroy();
        (void)this->_worlds.destroy();
        (void)this->_characters.destroy();
        (void)this->_variables.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    if (world->initialize() != FT_ERR_SUCCESS)
    {
        (void)this->_hooks.destroy();
        (void)this->_worlds.destroy();
        (void)this->_characters.destroy();
        (void)this->_variables.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(world->get_error());
        return (world->get_error());
    }
    this->_worlds.push_back(world);
    if (this->_worlds.get_error() != FT_ERR_SUCCESS)
    {
        move_error = this->_worlds.get_error();
        (void)this->_hooks.destroy();
        (void)this->_worlds.destroy();
        (void)this->_characters.destroy();
        (void)this->_variables.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(move_error);
        return (move_error);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_state::move(game_state &other) noexcept
{
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t source_destroy_error;
    int32_t source_error;
    int32_t move_error;
    ft_size_t index;
    ft_size_t count;

    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_state::move",
            "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
        {
            this->set_error(destroy_error);
            return (destroy_error);
        }
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
        {
            this->set_error(destroy_error);
            return (destroy_error);
        }
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_worlds.clear();
    index = 0;
    count = other._worlds.size();
    while (index < count)
    {
        this->_worlds.push_back(other._worlds[index]);
        if (this->_worlds.get_error() != FT_ERR_SUCCESS)
        {
            move_error = this->_worlds.get_error();
            this->set_error(move_error);
            (void)this->destroy();
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            this->set_error(move_error);
            return (move_error);
        }
        index++;
    }
    this->_characters.clear();
    index = 0;
    count = other._characters.size();
    while (index < count)
    {
        this->_characters.push_back(other._characters[index]);
        if (this->_characters.get_error() != FT_ERR_SUCCESS)
        {
            move_error = this->_characters.get_error();
            this->set_error(move_error);
            (void)this->destroy();
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            this->set_error(move_error);
            return (move_error);
        }
        index++;
    }
    this->_variables = other._variables;
    if (this->_variables.get_error() != FT_ERR_SUCCESS)
    {
        move_error = this->_variables.get_error();
        this->set_error(move_error);
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(move_error);
        return (move_error);
    }
    this->_hooks = other._hooks;
    if (this->_hooks.get_error() != FT_ERR_SUCCESS)
    {
        move_error = this->_hooks.get_error();
        this->set_error(move_error);
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(move_error);
        return (move_error);
    }
    source_error = other.get_error();
    source_destroy_error = other.destroy();
    if (source_destroy_error != FT_ERR_SUCCESS)
    {
        this->set_error(source_destroy_error);
        return (source_destroy_error);
    }
    this->set_error(source_error);
    return (FT_ERR_SUCCESS);
}

int32_t game_state::destroy() noexcept
{
    int32_t worlds_error;
    int32_t characters_error;
    int32_t variables_error;
    int32_t hooks_error;
    int32_t disable_error;
    int32_t final_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    worlds_error = this->_worlds.destroy();
    characters_error = this->_characters.destroy();
    variables_error = this->_variables.destroy();
    hooks_error = this->_hooks.destroy();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    final_error = disable_error;
    if (final_error == FT_ERR_SUCCESS && worlds_error != FT_ERR_SUCCESS)
        final_error = worlds_error;
    if (final_error == FT_ERR_SUCCESS && characters_error != FT_ERR_SUCCESS)
        final_error = characters_error;
    if (final_error == FT_ERR_SUCCESS && variables_error != FT_ERR_SUCCESS)
        final_error = variables_error;
    if (final_error == FT_ERR_SUCCESS && hooks_error != FT_ERR_SUCCESS)
        final_error = hooks_error;
    this->set_error(final_error);
    return (final_error);
}

int32_t game_state::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void game_state::unlock_internal(ft_bool lock_acquired) const noexcept
{

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::unlock_internal");
    if (lock_acquired == FT_FALSE)
        return ;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

ft_vector<ft_sharedptr<game_world> > &game_state::get_worlds() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::get_worlds");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_worlds);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_worlds);
}

ft_vector<ft_sharedptr<game_character> > &game_state::get_characters() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::get_characters");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_characters);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_characters);
}

void game_state::set_variable(const ft_string &key, const ft_string &value) noexcept
{
    Pair<ft_string, ft_string> *entry;
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::set_variable");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    entry = this->_variables.find(key);
    if (entry != this->_variables.end())
    {
        entry->value = value;
        if (entry->value.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(entry->value.get_error());
            this->unlock_internal(lock_acquired);
            return ;
        }
    }
    else
    {
        this->_variables.insert(key, value);
        if (this->_variables.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(this->_variables.get_error());
            this->unlock_internal(lock_acquired);
            return ;
        }
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_string *game_state::get_variable(const ft_string &key) const noexcept
{
    const Pair<ft_string, ft_string> *entry;
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::get_variable");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<game_state *>(this)->set_error(lock_error);
        return (ft_nullptr);
    }
    entry = this->_variables.find(key);
    if (entry == this->_variables.end())
    {
        const_cast<game_state *>(this)->set_error(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return (ft_nullptr);
    }
    const_cast<game_state *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (&entry->value);
}

void game_state::remove_variable(const ft_string &key) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::remove_variable");
    lock_acquired = FT_FALSE;
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

void game_state::clear_variables() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::clear_variables");
    lock_acquired = FT_FALSE;
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

int32_t game_state::add_character(const ft_sharedptr<game_character> &character) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::add_character");
    lock_acquired = FT_FALSE;
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
    this->_characters.push_back(character);
    if (this->_characters.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_characters.get_error());
        this->unlock_internal(lock_acquired);
        return (this->get_error());
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->get_error());
}

void game_state::remove_character(ft_size_t index) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::remove_character");
    lock_acquired = FT_FALSE;
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

void game_state::set_hooks(const ft_sharedptr<game_hooks> &hooks) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::set_hooks");
    lock_acquired = FT_FALSE;
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

static void game_state_delete_hooks_handle(ft_sharedptr<game_hooks> *hooks) noexcept
{
    if (hooks == ft_nullptr)
        return ;
    (void)hooks->destroy();
    delete hooks;
    return ;
}

ft_sharedptr<game_hooks> *game_state::get_hooks() const noexcept
{
    ft_sharedptr<game_hooks> *hooks_copy;
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t hooks_initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::get_hooks");
    hooks_copy = new (std::nothrow) ft_sharedptr<game_hooks>();
    if (hooks_copy == ft_nullptr)
    {
        const_cast<game_state *>(this)->set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    hooks_initialize_error = hooks_copy->initialize();
    if (hooks_initialize_error != FT_ERR_SUCCESS)
    {
        const_cast<game_state *>(this)->set_error(hooks_initialize_error);
        delete hooks_copy;
        return (ft_nullptr);
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<game_state *>(this)->set_error(lock_error);
        game_state_delete_hooks_handle(hooks_copy);
        return (ft_nullptr);
    }
    *hooks_copy = this->_hooks;
    if (hooks_copy->get_error() != FT_ERR_SUCCESS)
    {
        const_cast<game_state *>(this)->set_error(hooks_copy->get_error());
        this->unlock_internal(lock_acquired);
        game_state_delete_hooks_handle(hooks_copy);
        return (ft_nullptr);
    }
    const_cast<game_state *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (hooks_copy);
}

void game_state::reset_hooks() noexcept
{
    ft_sharedptr<game_hooks> hooks_copy;
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t hooks_initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::reset_hooks");
    hooks_initialize_error = hooks_copy.initialize();
    if (hooks_initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(hooks_initialize_error);
        return ;
    }
    lock_acquired = FT_FALSE;
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

void game_state::dispatch_item_crafted(game_character &character, game_item &item) const noexcept
{
    ft_sharedptr<game_hooks> hooks_copy;
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t hooks_initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::dispatch_item_crafted");
    hooks_initialize_error = hooks_copy.initialize();
    if (hooks_initialize_error != FT_ERR_SUCCESS)
    {
        const_cast<game_state *>(this)->set_error(hooks_initialize_error);
        return ;
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<game_state *>(this)->set_error(lock_error);
        return ;
    }
    hooks_copy = this->_hooks;
    const_cast<game_state *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    if (hooks_copy)
        hooks_copy->invoke_on_item_crafted(character, item);
    return ;
}

void game_state::dispatch_character_damaged(game_character &character, int32_t damage, uint8_t type) const noexcept
{
    ft_sharedptr<game_hooks> hooks_copy;
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t hooks_initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::dispatch_character_damaged");
    hooks_initialize_error = hooks_copy.initialize();
    if (hooks_initialize_error != FT_ERR_SUCCESS)
    {
        const_cast<game_state *>(this)->set_error(hooks_initialize_error);
        return ;
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<game_state *>(this)->set_error(lock_error);
        return ;
    }
    hooks_copy = this->_hooks;
    const_cast<game_state *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    if (hooks_copy)
        hooks_copy->invoke_on_character_damaged(character, damage, type);
    return ;
}

void game_state::dispatch_event_triggered(game_world &world, game_event &event) const noexcept
{
    ft_sharedptr<game_hooks> hooks_copy;
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t hooks_initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::dispatch_event_triggered");
    hooks_initialize_error = hooks_copy.initialize();
    if (hooks_initialize_error != FT_ERR_SUCCESS)
    {
        const_cast<game_state *>(this)->set_error(hooks_initialize_error);
        return ;
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<game_state *>(this)->set_error(lock_error);
        return ;
    }
    hooks_copy = this->_hooks;
    const_cast<game_state *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    if (hooks_copy)
        hooks_copy->invoke_on_event_triggered(world, event);
    return ;
}

int32_t game_state::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_state::get_error");
    return (game_state::_last_error);
}

const char *game_state::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_state::get_error_str");
    return (ft_strerror(game_state::_last_error));
}

int32_t game_state::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::enable_thread_safety");
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

int32_t game_state::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

ft_bool game_state::is_thread_safe() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_state::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int32_t game_state::set_error(int32_t error_code) noexcept
{
    game_state::_last_error = error_code;
    return (error_code);
}
