 #include "game_hooks.hpp"
 #include "../Printf/printf.hpp"
 #include "../System_utils/system_utils.hpp"
 #include "../Template/move.hpp"
 #include <new>

thread_local int ft_game_hooks::_last_error = FT_ERR_SUCCESS;

void ft_game_hooks::set_error(int error_code) const noexcept
{
    ft_game_hooks::_last_error = error_code;
    return ;
}

int ft_game_hooks::get_error() const noexcept
{
    return (ft_game_hooks::_last_error);
}

const char *ft_game_hooks::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

const char *ft_game_hook_item_crafted_identifier = "game.item.crafted";
const char *ft_game_hook_character_damaged_identifier = "game.character.damaged";
const char *ft_game_hook_event_triggered_identifier = "game.event.triggered";

ft_function<void(ft_game_hook_context&)> ft_game_hook_make_character_item_adapter(ft_function<void(ft_character&, ft_item&)> &&callback) noexcept
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

ft_function<void(ft_game_hook_context&)> ft_game_hook_make_character_damage_adapter(ft_function<void(ft_character&, int, uint8_t)> &&callback) noexcept
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

ft_function<void(ft_game_hook_context&)> ft_game_hook_make_world_event_adapter(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept
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

ft_game_hooks::ft_game_hooks() noexcept
    : _legacy_item_crafted(), _legacy_character_damaged(), _legacy_event_triggered(),
      _listener_catalog(), _catalog_metadata(), _mutex(ft_nullptr),
      _initialized_state(ft_game_hooks::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_game_hooks::~ft_game_hooks() noexcept
{
    if (this->_initialized_state == ft_game_hooks::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_game_hooks::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_game_hooks::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_game_hooks lifecycle error: %s: %s\n", method_name,
        reason);
    su_abort();
    return ;
}

void ft_game_hooks::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_game_hooks::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_game_hooks::initialize() noexcept
{
    if (this->_initialized_state == ft_game_hooks::_state_initialized)
    {
        this->abort_lifecycle_error("ft_game_hooks::initialize",
            "called while object is already initialized");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_legacy_item_crafted = ft_function<void(ft_character&, ft_item&)>();
    this->_legacy_character_damaged = ft_function<void(ft_character&, int, uint8_t)>();
    this->_legacy_event_triggered = ft_function<void(ft_world&, ft_event&)>();
    this->_listener_catalog.clear();
    this->_catalog_metadata.clear();
    this->_initialized_state = ft_game_hooks::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_game_hooks::initialize(const ft_game_hooks &other) noexcept
{
    int initialize_error;
    size_t count;
    size_t index;
    const Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *entry;
    const Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *entry_end;
    const ft_game_hook_metadata *metadata_entry;
    const ft_game_hook_metadata *metadata_end;

    if (other._initialized_state != ft_game_hooks::_state_initialized)
    {
        other.abort_lifecycle_error("ft_game_hooks::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
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

int ft_game_hooks::initialize(ft_game_hooks &&other) noexcept
{
    int result = this->initialize(static_cast<const ft_game_hooks &>(other));
    this->set_error(result);
    return (result);
}

int ft_game_hooks::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_game_hooks::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_legacy_item_crafted = ft_function<void(ft_character&, ft_item&)>();
    this->_legacy_character_damaged = ft_function<void(ft_character&, int, uint8_t)>();
    this->_legacy_event_triggered = ft_function<void(ft_world&, ft_event&)>();
    this->_listener_catalog.clear();
    this->_catalog_metadata.clear();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_game_hooks::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_game_hooks::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_game_hooks::enable_thread_safety");
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

int ft_game_hooks::disable_thread_safety() noexcept
{
    int destroy_error;

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

bool ft_game_hooks::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_game_hooks::is_thread_safe");
    bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_game_hooks::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        return (FT_ERR_SUCCESS);
    }
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_game_hooks::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_game_hooks::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_game_hooks::lock");
    int lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_game_hooks::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_game_hooks::unlock");
    int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    (void)unlock_result;
    return ;
}

void ft_game_hooks::remove_listener_unlocked(const ft_string &hook_identifier,
    const ft_string &listener_name) noexcept
{
    Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *listeners_pair;
    size_t index;

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

void ft_game_hooks::insert_listener_unlocked(
    const ft_game_hook_listener_entry &entry) noexcept
{
    Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *listeners_pair;
    ft_vector<ft_game_hook_listener_entry> listeners;
    size_t index;

    listeners_pair = this->_listener_catalog.find(entry.metadata.hook_identifier);
    if (listeners_pair == this->_listener_catalog.end())
    {
        listeners.clear();
        listeners.push_back(entry);
        this->_listener_catalog.insert(entry.metadata.hook_identifier, listeners);
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

void ft_game_hooks::append_metadata_unlocked(
    const ft_game_hook_metadata &metadata) noexcept
{
    this->_catalog_metadata.push_back(metadata);
    return ;
}

void ft_game_hooks::set_on_item_crafted(
    ft_function<void(ft_character&, ft_item&)> &&callback) noexcept
{
    ft_game_hook_listener_entry entry;
    bool lock_acquired;

    this->abort_if_not_initialized("ft_game_hooks::set_on_item_crafted");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_legacy_item_crafted = ft_move(callback);
    entry.metadata.hook_identifier = ft_game_hook_item_crafted_identifier;
    entry.metadata.listener_name = "legacy.item_crafted";
    entry.metadata.description = "Legacy callback set via set_on_item_crafted";
    entry.metadata.argument_contract = "ft_character&,ft_item&";
    entry.priority = 1000;
    entry.callback = ft_game_hook_make_character_item_adapter(
        ft_move(this->_legacy_item_crafted));
    this->remove_listener_unlocked(entry.metadata.hook_identifier,
        entry.metadata.listener_name);
    this->insert_listener_unlocked(entry);
    this->append_metadata_unlocked(entry.metadata);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_game_hooks::set_on_character_damaged(
    ft_function<void(ft_character&, int, uint8_t)> &&callback) noexcept
{
    ft_game_hook_listener_entry entry;
    bool lock_acquired;

    this->abort_if_not_initialized("ft_game_hooks::set_on_character_damaged");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_legacy_character_damaged = ft_move(callback);
    entry.metadata.hook_identifier = ft_game_hook_character_damaged_identifier;
    entry.metadata.listener_name = "legacy.character_damaged";
    entry.metadata.description = "Legacy callback set via set_on_character_damaged";
    entry.metadata.argument_contract = "ft_character&,int,uint8_t";
    entry.priority = 1000;
    entry.callback = ft_game_hook_make_character_damage_adapter(
        ft_move(this->_legacy_character_damaged));
    this->remove_listener_unlocked(entry.metadata.hook_identifier,
        entry.metadata.listener_name);
    this->insert_listener_unlocked(entry);
    this->append_metadata_unlocked(entry.metadata);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_game_hooks::set_on_event_triggered(
    ft_function<void(ft_world&, ft_event&)> &&callback) noexcept
{
    ft_game_hook_listener_entry entry;
    bool lock_acquired;

    this->abort_if_not_initialized("ft_game_hooks::set_on_event_triggered");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_legacy_event_triggered = ft_move(callback);
    entry.metadata.hook_identifier = ft_game_hook_event_triggered_identifier;
    entry.metadata.listener_name = "legacy.event_triggered";
    entry.metadata.description = "Legacy callback set via set_on_event_triggered";
    entry.metadata.argument_contract = "ft_world&,ft_event&";
    entry.priority = 1000;
    entry.callback = ft_game_hook_make_world_event_adapter(
        ft_move(this->_legacy_event_triggered));
    this->remove_listener_unlocked(entry.metadata.hook_identifier,
        entry.metadata.listener_name);
    this->insert_listener_unlocked(entry);
    this->append_metadata_unlocked(entry.metadata);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_function<void(ft_character&, ft_item&)> ft_game_hooks::get_on_item_crafted() const noexcept
{
    this->abort_if_not_initialized("ft_game_hooks::get_on_item_crafted");
    return (this->_legacy_item_crafted);
}

ft_function<void(ft_character&, int, uint8_t)> ft_game_hooks::get_on_character_damaged() const noexcept
{
    this->abort_if_not_initialized("ft_game_hooks::get_on_character_damaged");
    return (this->_legacy_character_damaged);
}

ft_function<void(ft_world&, ft_event&)> ft_game_hooks::get_on_event_triggered() const noexcept
{
    this->abort_if_not_initialized("ft_game_hooks::get_on_event_triggered");
    return (this->_legacy_event_triggered);
}

void ft_game_hooks::invoke_hook(const ft_string &hook_identifier,
    ft_game_hook_context &context) const noexcept
{
    const Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *listeners_pair;
    size_t index;

    this->abort_if_not_initialized("ft_game_hooks::invoke_hook");
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

void ft_game_hooks::invoke_on_item_crafted(ft_character &character,
    ft_item &item) const noexcept
{
    ft_game_hook_context context;

    context.world = ft_nullptr;
    context.character = &character;
    context.item = &item;
    context.event = ft_nullptr;
    context.integer_payload = 0;
    context.small_payload = 0;
    this->invoke_hook(ft_string(ft_game_hook_item_crafted_identifier), context);
    return ;
}

void ft_game_hooks::invoke_on_character_damaged(ft_character &character,
    int damage, uint8_t type) const noexcept
{
    ft_game_hook_context context;

    context.world = ft_nullptr;
    context.character = &character;
    context.item = ft_nullptr;
    context.event = ft_nullptr;
    context.integer_payload = damage;
    context.small_payload = type;
    this->invoke_hook(ft_string(ft_game_hook_character_damaged_identifier), context);
    return ;
}

void ft_game_hooks::invoke_on_event_triggered(ft_world &world,
    ft_event &event) const noexcept
{
    ft_game_hook_context context;

    context.world = &world;
    context.character = ft_nullptr;
    context.item = ft_nullptr;
    context.event = &event;
    context.integer_payload = 0;
    context.small_payload = 0;
    this->invoke_hook(ft_string(ft_game_hook_event_triggered_identifier), context);
    return ;
}

void ft_game_hooks::register_listener(const ft_game_hook_metadata &metadata,
    int priority, ft_function<void(ft_game_hook_context&)> &&callback) noexcept
{
    ft_game_hook_listener_entry entry;
    bool lock_acquired;

    this->abort_if_not_initialized("ft_game_hooks::register_listener");
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

void ft_game_hooks::unregister_listener(const ft_string &hook_identifier,
    const ft_string &listener_name) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_game_hooks::unregister_listener");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->remove_listener_unlocked(hook_identifier, listener_name);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

ft_vector<ft_game_hook_metadata> ft_game_hooks::get_catalog_metadata() const noexcept
{
    this->abort_if_not_initialized("ft_game_hooks::get_catalog_metadata");
    return (ft_vector<ft_game_hook_metadata>());
}

ft_vector<ft_game_hook_metadata> ft_game_hooks::get_catalog_metadata_for(
    const ft_string &) const noexcept
{
    this->abort_if_not_initialized("ft_game_hooks::get_catalog_metadata_for");
    return (ft_vector<ft_game_hook_metadata>());
}

void ft_game_hooks::reset() noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_game_hooks::reset");
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_legacy_item_crafted = ft_function<void(ft_character&, ft_item&)>();
    this->_legacy_character_damaged = ft_function<void(ft_character&, int, uint8_t)>();
    this->_legacy_event_triggered = ft_function<void(ft_world&, ft_event&)>();
    this->_listener_catalog.clear();
    this->_catalog_metadata.clear();
    (void)this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_game_hooks::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_game_hooks::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
