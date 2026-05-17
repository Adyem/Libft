#include "../PThread/pthread_internal.hpp"
 #include "game_hooks.hpp"
 #include "../Printf/printf.hpp"
 #include "../System_utils/system_utils.hpp"
 #include "../Errno/errno_internal.hpp"
 #include "../Template/move.hpp"
 #include <new>

thread_local int32_t game_hooks::_last_error = FT_ERR_SUCCESS;

int32_t game_hooks::set_error(int32_t error_code) noexcept
{
    game_hooks::_last_error = error_code;
    return (error_code);
}

int32_t game_hooks::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_hooks::get_error");
    return (game_hooks::_last_error);
}

const char *game_hooks::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_hooks::get_error_str");
    return (ft_strerror(game_hooks::_last_error));
}

const char *ft_game_hook_item_crafted_identifier = "game.item.crafted";
const char *ft_game_hook_character_damaged_identifier = "game.character.damaged";
const char *ft_game_hook_event_triggered_identifier = "game.event.triggered";

ft_function<void(ft_game_hook_context&)> ft_game_hook_make_character_item_adapter(ft_function<void(game_character&, game_item&)> &&callback) noexcept
{
    return (ft_function<void(ft_game_hook_context&)>([inner_callback = ft_move(callback)](ft_game_hook_context &context) mutable
    {
        if (!inner_callback)
            return ;
        if (context.character == ft_nullptr || context.item == ft_nullptr)
            return ;
        inner_callback(*context.character, *context.item);
        return ;
    }));
}

ft_function<void(ft_game_hook_context&)> ft_game_hook_make_character_damage_adapter(ft_function<void(game_character&, int32_t, uint8_t)> &&callback) noexcept
{
    return (ft_function<void(ft_game_hook_context&)>([inner_callback = ft_move(callback)](ft_game_hook_context &context) mutable
    {
        if (!inner_callback)
            return ;
        if (context.character == ft_nullptr)
            return ;
        inner_callback(*context.character, context.integer_payload, context.small_payload);
        return ;
    }));
}

ft_function<void(ft_game_hook_context&)> ft_game_hook_make_world_event_adapter(ft_function<void(game_world&, game_event&)> &&callback) noexcept
{
    return (ft_function<void(ft_game_hook_context&)>([inner_callback = ft_move(callback)](ft_game_hook_context &context) mutable
    {
        if (!inner_callback)
            return ;
        if (context.world == ft_nullptr || context.event == ft_nullptr)
            return ;
        inner_callback(*context.world, *context.event);
        return ;
    }));
}

game_hooks::game_hooks() noexcept
    : _legacy_item_crafted(), _legacy_character_damaged(), _legacy_event_triggered(),
      _listener_catalog(), _catalog_metadata(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_hooks::~game_hooks() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        return ;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t game_hooks::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_hooks::initialize",
            "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_legacy_item_crafted = ft_function<void(game_character&, game_item&)>();
    this->_legacy_character_damaged = ft_function<void(game_character&, int32_t, uint8_t)>();
    this->_legacy_event_triggered = ft_function<void(game_world&, game_event&)>();
    int32_t map_error = this->_listener_catalog.initialize();
    if (map_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(map_error);
        return (map_error);
    }
    int32_t metadata_error = this->_catalog_metadata.initialize();
    if (metadata_error != FT_ERR_SUCCESS)
    {
        (void)this->_listener_catalog.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(metadata_error);
        return (metadata_error);
    }
    this->_listener_catalog.clear();
    this->_catalog_metadata.clear();
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_hooks::initialize(const game_hooks &other) noexcept
{
    int32_t initialize_error;
    int32_t destroy_error;
    ft_size_t count;
    ft_size_t index;
    const Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *entry;
    const Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *entry_end;
    const ft_game_hook_metadata *metadata_entry;
    const ft_game_hook_metadata *metadata_end;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_hooks::initialize(copy)",
            "source object is uninitialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
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
        this->set_error(static_cast<uint32_t>(other.get_error()));
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
    this->_legacy_item_crafted = other._legacy_item_crafted;
    this->_legacy_character_damaged = other._legacy_character_damaged;
    this->_legacy_event_triggered = other._legacy_event_triggered;
    count = other._listener_catalog.size();
    entry_end = other._listener_catalog.end();
    entry = entry_end - count;
    index = 0;
    while (index < count)
    {
        this->_listener_catalog.insert(entry->key, entry->value);
        entry++;
        index += 1;
    }
    metadata_entry = other._catalog_metadata.begin();
    metadata_end = other._catalog_metadata.end();
    while (metadata_entry != metadata_end)
    {
    this->_catalog_metadata.push_back(*metadata_entry);
    ++metadata_entry;
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_hooks::initialize(game_hooks &&other) noexcept
{
    return (this->move(other));
}

int32_t game_hooks::move(game_hooks &other) noexcept
{
    int32_t initialize_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_hooks::move",
            "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    initialize_error = this->initialize(static_cast<const game_hooks &>(other));
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    if (other._initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)other.destroy();
    return (FT_ERR_SUCCESS);
}

int32_t game_hooks::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_legacy_item_crafted = ft_function<void(game_character&, game_item&)>();
    this->_legacy_character_damaged = ft_function<void(game_character&, int32_t, uint8_t)>();
    this->_legacy_event_triggered = ft_function<void(game_world&, game_event&)>();
    int32_t map_destroy_error = this->_listener_catalog.destroy();
    int32_t metadata_destroy_error = this->_catalog_metadata.destroy();
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    int32_t final_error = disable_error;
    if (final_error == FT_ERR_SUCCESS && map_destroy_error != FT_ERR_SUCCESS)
        final_error = map_destroy_error;
    if (final_error == FT_ERR_SUCCESS && metadata_destroy_error != FT_ERR_SUCCESS)
        final_error = metadata_destroy_error;
    this->set_error(final_error);
    return (final_error);
}

int32_t game_hooks::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
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

int32_t game_hooks::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

ft_bool game_hooks::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t game_hooks::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

int32_t game_hooks::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t game_hooks::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::lock");
    int32_t lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void game_hooks::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void game_hooks::remove_listener_unlocked(const ft_string &hook_identifier,
    const ft_string &listener_name) noexcept
{
    Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *listeners_pair;
    ft_size_t index;

    listeners_pair = this->_listener_catalog.find(hook_identifier);
    if (listeners_pair == this->_listener_catalog.end())
        return ;
    index = 0;
    while (index < listeners_pair->value.size())
    {
        if (listeners_pair->value[index].metadata.listener_name == listener_name)
        {
            listeners_pair->value.erase(listeners_pair->value.begin() + index);
            break ;
        }
        index += 1;
    }
    index = 0;
    while (index < this->_catalog_metadata.size())
    {
        if (this->_catalog_metadata[index].hook_identifier == hook_identifier
            && this->_catalog_metadata[index].listener_name == listener_name)
        {
            this->_catalog_metadata.erase(this->_catalog_metadata.begin() + index);
            break ;
        }
        index += 1;
    }
    return ;
}

void game_hooks::insert_listener_unlocked(
    const ft_game_hook_listener_entry &entry) noexcept
{
    Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *listeners_pair;
    ft_vector<ft_game_hook_listener_entry> listeners;
    ft_size_t index;
    int32_t error_code;

    listeners_pair = this->_listener_catalog.find(entry.metadata.hook_identifier);
    if (listeners_pair == this->_listener_catalog.end())
    {
        error_code = listeners.initialize();
        if (error_code != FT_ERR_SUCCESS)
        {
            this->set_error(listeners.get_error());
            return ;
        }
        listeners.clear();
        error_code = listeners.push_back(entry);
        if (error_code != FT_ERR_SUCCESS)
        {
            this->set_error(listeners.get_error());
            return ;
        }
        this->_listener_catalog.insert(entry.metadata.hook_identifier, listeners);
        this->set_error(this->_listener_catalog.get_error());
        return ;
    }
    index = 0;
    while (index < listeners_pair->value.size())
    {
        if (entry.priority > listeners_pair->value[index].priority)
            break ;
        index += 1;
    }
    listeners_pair->value.insert(listeners_pair->value.begin() + index, entry);
    return ;
}

void game_hooks::append_metadata_unlocked(
    const ft_game_hook_metadata &metadata) noexcept
{
    this->_catalog_metadata.push_back(metadata);
    return ;
}

void game_hooks::set_on_item_crafted(
    ft_function<void(game_character&, game_item&)> &&callback) noexcept
{
    ft_game_hook_listener_entry entry;
    ft_function<void(game_character&, game_item&)> callback_copy;
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::set_on_item_crafted");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_legacy_item_crafted = ft_move(callback);
    callback_copy = this->_legacy_item_crafted;
    entry.metadata.hook_identifier = ft_game_hook_item_crafted_identifier;
    entry.metadata.listener_name = "legacy.item_crafted";
    entry.metadata.description = "Legacy callback set via set_on_item_crafted";
    entry.metadata.argument_contract = "game_character&,game_item&";
    entry.priority = 1000;
    entry.callback = ft_game_hook_make_character_item_adapter(
        ft_move(callback_copy));
    this->remove_listener_unlocked(entry.metadata.hook_identifier,
        entry.metadata.listener_name);
    this->insert_listener_unlocked(entry);
    this->append_metadata_unlocked(entry.metadata);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void game_hooks::set_on_character_damaged(
    ft_function<void(game_character&, int32_t, uint8_t)> &&callback) noexcept
{
    ft_game_hook_listener_entry entry;
    ft_function<void(game_character&, int32_t, uint8_t)> callback_copy;
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::set_on_character_damaged");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_legacy_character_damaged = ft_move(callback);
    callback_copy = this->_legacy_character_damaged;
    entry.metadata.hook_identifier = ft_game_hook_character_damaged_identifier;
    entry.metadata.listener_name = "legacy.character_damaged";
    entry.metadata.description = "Legacy callback set via set_on_character_damaged";
    entry.metadata.argument_contract = "game_character&,int32_t,uint8_t";
    entry.priority = 1000;
    entry.callback = ft_game_hook_make_character_damage_adapter(
        ft_move(callback_copy));
    this->remove_listener_unlocked(entry.metadata.hook_identifier,
        entry.metadata.listener_name);
    this->insert_listener_unlocked(entry);
    this->append_metadata_unlocked(entry.metadata);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void game_hooks::set_on_event_triggered(
    ft_function<void(game_world&, game_event&)> &&callback) noexcept
{
    ft_game_hook_listener_entry entry;
    ft_function<void(game_world&, game_event&)> callback_copy;
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::set_on_event_triggered");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_legacy_event_triggered = ft_move(callback);
    callback_copy = this->_legacy_event_triggered;
    entry.metadata.hook_identifier = ft_game_hook_event_triggered_identifier;
    entry.metadata.listener_name = "legacy.event_triggered";
    entry.metadata.description = "Legacy callback set via set_on_event_triggered";
    entry.metadata.argument_contract = "game_world&,game_event&";
    entry.priority = 1000;
    entry.callback = ft_game_hook_make_world_event_adapter(
        ft_move(callback_copy));
    this->remove_listener_unlocked(entry.metadata.hook_identifier,
        entry.metadata.listener_name);
    this->insert_listener_unlocked(entry);
    this->append_metadata_unlocked(entry.metadata);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_function<void(game_character&, game_item&)> game_hooks::get_on_item_crafted() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::get_on_item_crafted");
    return (this->_legacy_item_crafted);
}

ft_function<void(game_character&, int32_t, uint8_t)> game_hooks::get_on_character_damaged() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::get_on_character_damaged");
    return (this->_legacy_character_damaged);
}

ft_function<void(game_world&, game_event&)> game_hooks::get_on_event_triggered() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::get_on_event_triggered");
    return (this->_legacy_event_triggered);
}

void game_hooks::invoke_hook(const ft_string &hook_identifier,
    ft_game_hook_context &context) const noexcept
{
    const Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *listeners_pair;
    ft_size_t index;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::invoke_hook");
    listeners_pair = this->_listener_catalog.find(hook_identifier);
    if (listeners_pair == this->_listener_catalog.end())
        return ;
    index = 0;
    while (index < listeners_pair->value.size())
    {
        if (listeners_pair->value[index].callback)
            listeners_pair->value[index].callback(context);
        index += 1;
    }
    return ;
}

void game_hooks::invoke_on_item_crafted(game_character &character,
    game_item &item) const noexcept
{
    ft_game_hook_context context;

    context.world = ft_nullptr;
    context.character = &character;
    context.item = &item;
    context.event = ft_nullptr;
    context.integer_payload = 0;
    context.small_payload = 0;
    ft_string hook_identifier;
    if (hook_identifier.initialize(ft_game_hook_item_crafted_identifier) != FT_ERR_SUCCESS)
        return ;
    this->invoke_hook(hook_identifier, context);
    return ;
}

void game_hooks::invoke_on_character_damaged(game_character &character,
    int32_t damage, uint8_t type) const noexcept
{
    ft_game_hook_context context;

    context.world = ft_nullptr;
    context.character = &character;
    context.item = ft_nullptr;
    context.event = ft_nullptr;
    context.integer_payload = damage;
    context.small_payload = type;
    ft_string hook_identifier;
    if (hook_identifier.initialize(ft_game_hook_character_damaged_identifier) != FT_ERR_SUCCESS)
        return ;
    this->invoke_hook(hook_identifier, context);
    return ;
}

void game_hooks::invoke_on_event_triggered(game_world &world,
    game_event &event) const noexcept
{
    ft_game_hook_context context;

    context.world = &world;
    context.character = ft_nullptr;
    context.item = ft_nullptr;
    context.event = &event;
    context.integer_payload = 0;
    context.small_payload = 0;
    ft_string hook_identifier;
    if (hook_identifier.initialize(ft_game_hook_event_triggered_identifier) != FT_ERR_SUCCESS)
        return ;
    this->invoke_hook(hook_identifier, context);
    return ;
}

void game_hooks::register_listener(const ft_game_hook_metadata &metadata,
    int32_t priority, ft_function<void(ft_game_hook_context&)> &&callback) noexcept
{
    ft_game_hook_listener_entry entry;
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::register_listener");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    entry.metadata = metadata;
    entry.priority = priority;
    entry.callback = ft_move(callback);
    this->remove_listener_unlocked(entry.metadata.hook_identifier,
        entry.metadata.listener_name);
    this->insert_listener_unlocked(entry);
    this->append_metadata_unlocked(entry.metadata);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void game_hooks::unregister_listener(const ft_string &hook_identifier,
    const ft_string &listener_name) noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::unregister_listener");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->remove_listener_unlocked(hook_identifier, listener_name);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

static void game_hooks_delete_metadata_vector(
    ft_vector<ft_game_hook_metadata> *metadata) noexcept
{
    if (metadata == ft_nullptr)
        return ;
    (void)metadata->destroy();
    delete metadata;
    return ;
}

ft_vector<ft_game_hook_metadata> *game_hooks::get_catalog_metadata() const noexcept
{
    ft_vector<ft_game_hook_metadata> *result;
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_size_t index;
    ft_size_t metadata_count;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::get_catalog_metadata");
    result = new (std::nothrow) ft_vector<ft_game_hook_metadata>();
    if (result == ft_nullptr)
    {
        const_cast<game_hooks *>(this)->set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    if (result->initialize() != FT_ERR_SUCCESS)
    {
        const_cast<game_hooks *>(this)->set_error(result->get_error());
        delete result;
        return (ft_nullptr);
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        game_hooks_delete_metadata_vector(result);
        return (ft_nullptr);
    }
    index = 0;
    metadata_count = this->_catalog_metadata.size();
    while (index < metadata_count)
    {
        if (result->push_back(this->_catalog_metadata[index]) != FT_ERR_SUCCESS)
        {
            const_cast<game_hooks *>(this)->set_error(result->get_error());
            (void)this->unlock_internal(lock_acquired);
            game_hooks_delete_metadata_vector(result);
            return (ft_nullptr);
        }
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    const_cast<game_hooks *>(this)->set_error(FT_ERR_SUCCESS);
    return (result);
}

ft_vector<ft_game_hook_metadata> *game_hooks::get_catalog_metadata_for(
    const ft_string &hook_identifier) const noexcept
{
    ft_vector<ft_game_hook_metadata> *result;
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_size_t index;
    ft_size_t metadata_count;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::get_catalog_metadata_for");
    result = new (std::nothrow) ft_vector<ft_game_hook_metadata>();
    if (result == ft_nullptr)
    {
        const_cast<game_hooks *>(this)->set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    if (result->initialize() != FT_ERR_SUCCESS)
    {
        const_cast<game_hooks *>(this)->set_error(result->get_error());
        delete result;
        return (ft_nullptr);
    }
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        game_hooks_delete_metadata_vector(result);
        return (ft_nullptr);
    }
    index = 0;
    metadata_count = this->_catalog_metadata.size();
    while (index < metadata_count)
    {
        if (this->_catalog_metadata[index].hook_identifier == hook_identifier)
        {
            if (result->push_back(this->_catalog_metadata[index]) != FT_ERR_SUCCESS)
            {
                const_cast<game_hooks *>(this)->set_error(result->get_error());
                (void)this->unlock_internal(lock_acquired);
                game_hooks_delete_metadata_vector(result);
                return (ft_nullptr);
            }
        }
        index += 1;
    }
    (void)this->unlock_internal(lock_acquired);
    const_cast<game_hooks *>(this)->set_error(FT_ERR_SUCCESS);
    return (result);
}

void game_hooks::reset() noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_hooks::reset");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_legacy_item_crafted = ft_function<void(game_character&, game_item&)>();
    this->_legacy_character_damaged = ft_function<void(game_character&, int32_t, uint8_t)>();
    this->_legacy_event_triggered = ft_function<void(game_world&, game_event&)>();
    this->_listener_catalog.clear();
    this->_catalog_metadata.clear();
    (void)this->unlock_internal(lock_acquired);
    return ;
}
