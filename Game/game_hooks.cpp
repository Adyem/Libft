#include "game_hooks.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

const char *ft_game_hook_item_crafted_identifier = "game.item.crafted";
const char *ft_game_hook_character_damaged_identifier = "game.character.damaged";
const char *ft_game_hook_event_triggered_identifier = "game.event.triggered";

static void game_hooks_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_hooks_unlock_guard(ft_unique_lock<pt_mutex> &guard)
{
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

ft_function<void(ft_game_hook_context&)> ft_game_hook_make_character_item_adapter(ft_function<void(ft_character&, ft_item&)> &&callback) noexcept
{
    ft_function<void(ft_game_hook_context&)> adapter;

    adapter = ft_function<void(ft_game_hook_context&)>();
    adapter = ft_function<void(ft_game_hook_context&)>([inner_callback = ft_move(callback)](ft_game_hook_context &context) mutable
    {
        if (!inner_callback)
            return ;
        if (context.character == ft_nullptr)
            return ;
        if (context.item == ft_nullptr)
            return ;
        inner_callback(*context.character, *context.item);
        return ;
    });
    return (adapter);
}

ft_function<void(ft_game_hook_context&)> ft_game_hook_make_character_damage_adapter(ft_function<void(ft_character&, int, uint8_t)> &&callback) noexcept
{
    ft_function<void(ft_game_hook_context&)> adapter;

    adapter = ft_function<void(ft_game_hook_context&)>();
    adapter = ft_function<void(ft_game_hook_context&)>([inner_callback = ft_move(callback)](ft_game_hook_context &context) mutable
    {
        if (!inner_callback)
            return ;
        if (context.character == ft_nullptr)
            return ;
        inner_callback(*context.character, context.integer_payload, context.small_payload);
        return ;
    });
    return (adapter);
}

ft_function<void(ft_game_hook_context&)> ft_game_hook_make_world_event_adapter(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept
{
    ft_function<void(ft_game_hook_context&)> adapter;

    adapter = ft_function<void(ft_game_hook_context&)>();
    adapter = ft_function<void(ft_game_hook_context&)>([inner_callback = ft_move(callback)](ft_game_hook_context &context) mutable
    {
        if (!inner_callback)
            return ;
        if (context.world == ft_nullptr)
            return ;
        if (context.event == ft_nullptr)
            return ;
        inner_callback(*context.world, *context.event);
        return ;
    });
    return (adapter);
}

int ft_game_hooks::lock_pair(const ft_game_hooks &first, const ft_game_hooks &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_game_hooks *ordered_first;
    const ft_game_hooks *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESSS;
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_game_hooks *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESSS)
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
            ft_errno = FT_ERR_SUCCESSS;
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_hooks_sleep_backoff();
    }
}

ft_game_hooks::ft_game_hooks() noexcept
    : _legacy_item_crafted(), _legacy_character_damaged(), _legacy_event_triggered(),
      _listener_catalog(), _catalog_metadata(), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_game_hooks::~ft_game_hooks() noexcept
{
    return ;
}

ft_game_hooks::ft_game_hooks(const ft_game_hooks &other) noexcept
    : _legacy_item_crafted(), _legacy_character_damaged(), _legacy_event_triggered(),
      _listener_catalog(), _catalog_metadata(), _error_code(other._error_code), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->_legacy_item_crafted = other._legacy_item_crafted;
    if (this->_legacy_item_crafted.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_item_crafted.get_error());
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->_legacy_character_damaged = other._legacy_character_damaged;
    if (this->_legacy_character_damaged.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_character_damaged.get_error());
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->_legacy_event_triggered = other._legacy_event_triggered;
    if (this->_legacy_event_triggered.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_event_triggered.get_error());
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->_listener_catalog = ft_map<ft_string, ft_vector<ft_game_hook_listener_entry> >();
    if (this->_listener_catalog.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_listener_catalog.get_error());
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->_catalog_metadata = ft_vector<ft_game_hook_metadata>();
    if (this->_catalog_metadata.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_catalog_metadata.get_error());
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->clone_catalog_from(other);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    game_hooks_unlock_guard(other_guard);
    return ;
}

ft_game_hooks &ft_game_hooks::operator=(const ft_game_hooks &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_game_hooks::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_legacy_item_crafted = other._legacy_item_crafted;
    if (this->_legacy_item_crafted.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_item_crafted.get_error());
        return (*this);
    }
    this->_legacy_character_damaged = other._legacy_character_damaged;
    if (this->_legacy_character_damaged.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_character_damaged.get_error());
        return (*this);
    }
    this->_legacy_event_triggered = other._legacy_event_triggered;
    if (this->_legacy_event_triggered.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_event_triggered.get_error());
        return (*this);
    }
    this->_listener_catalog.clear();
    if (this->_listener_catalog.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_listener_catalog.get_error());
        return (*this);
    }
    this->_catalog_metadata.clear();
    if (this->_catalog_metadata.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_catalog_metadata.get_error());
        return (*this);
    }
    this->clone_catalog_from(other);
    if (this->_error_code != FT_ERR_SUCCESSS)
        return (*this);
    this->_error_code = other._error_code;
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

ft_game_hooks::ft_game_hooks(ft_game_hooks &&other) noexcept
    : _legacy_item_crafted(), _legacy_character_damaged(), _legacy_event_triggered(),
      _listener_catalog(), _catalog_metadata(), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->_legacy_item_crafted = ft_move(other._legacy_item_crafted);
    if (this->_legacy_item_crafted.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_item_crafted.get_error());
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->_legacy_character_damaged = ft_move(other._legacy_character_damaged);
    if (this->_legacy_character_damaged.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_character_damaged.get_error());
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->_legacy_event_triggered = ft_move(other._legacy_event_triggered);
    if (this->_legacy_event_triggered.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_event_triggered.get_error());
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->_listener_catalog = ft_move(other._listener_catalog);
    if (this->_listener_catalog.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_listener_catalog.get_error());
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->_catalog_metadata = ft_move(other._catalog_metadata);
    if (this->_catalog_metadata.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_catalog_metadata.get_error());
        game_hooks_unlock_guard(other_guard);
        return ;
    }
    this->_error_code = other._error_code;
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    game_hooks_unlock_guard(other_guard);
    return ;
}

ft_game_hooks &ft_game_hooks::operator=(ft_game_hooks &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_game_hooks::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_legacy_item_crafted = ft_move(other._legacy_item_crafted);
    if (this->_legacy_item_crafted.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_item_crafted.get_error());
        return (*this);
    }
    this->_legacy_character_damaged = ft_move(other._legacy_character_damaged);
    if (this->_legacy_character_damaged.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_character_damaged.get_error());
        return (*this);
    }
    this->_legacy_event_triggered = ft_move(other._legacy_event_triggered);
    if (this->_legacy_event_triggered.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_event_triggered.get_error());
        return (*this);
    }
    this->_listener_catalog = ft_move(other._listener_catalog);
    if (this->_listener_catalog.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_listener_catalog.get_error());
        return (*this);
    }
    this->_catalog_metadata = ft_move(other._catalog_metadata);
    if (this->_catalog_metadata.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_catalog_metadata.get_error());
        return (*this);
    }
    this->_error_code = other._error_code;
    other._error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

void ft_game_hooks::remove_listener_unlocked(const ft_string &hook_identifier, const ft_string &listener_name) noexcept
{
    Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *listeners_pair;
    size_t index;

    listeners_pair = this->_listener_catalog.find(hook_identifier);
    if (this->_listener_catalog.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_listener_catalog.get_error());
        return ;
    }
    if (listeners_pair == this->_listener_catalog.end())
    {
        this->set_error(FT_ERR_SUCCESSS);
        return ;
    }
    index = 0;
    while (index < listeners_pair->value.size())
    {
        if (listeners_pair->value[index].metadata.listener_name == listener_name)
        {
            listeners_pair->value.erase(listeners_pair->value.begin() + index);
            if (listeners_pair->value.get_error() != FT_ERR_SUCCESSS)
            {
                this->set_error(listeners_pair->value.get_error());
                return ;
            }
            break ;
        }
        index++;
    }
    size_t catalog_index;

    catalog_index = 0;
    while (catalog_index < this->_catalog_metadata.size())
    {
        if (this->_catalog_metadata[catalog_index].hook_identifier == hook_identifier &&
            this->_catalog_metadata[catalog_index].listener_name == listener_name)
        {
            this->_catalog_metadata.erase(this->_catalog_metadata.begin() + catalog_index);
            if (this->_catalog_metadata.get_error() != FT_ERR_SUCCESSS)
            {
                this->set_error(this->_catalog_metadata.get_error());
                return ;
            }
            break ;
        }
        catalog_index++;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_game_hooks::insert_listener_unlocked(const ft_game_hook_listener_entry &entry) noexcept
{
    Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *listeners_pair;
    ft_vector<ft_game_hook_listener_entry> new_listeners;
    size_t index;

    listeners_pair = this->_listener_catalog.find(entry.metadata.hook_identifier);
    if (this->_listener_catalog.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_listener_catalog.get_error());
        return ;
    }
    if (listeners_pair == this->_listener_catalog.end())
    {
        new_listeners = ft_vector<ft_game_hook_listener_entry>();
        new_listeners.push_back(entry);
        if (new_listeners.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(new_listeners.get_error());
            return ;
        }
        this->_listener_catalog.insert(entry.metadata.hook_identifier, ft_move(new_listeners));
        if (this->_listener_catalog.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(this->_listener_catalog.get_error());
            return ;
        }
        this->set_error(FT_ERR_SUCCESSS);
        return ;
    }
    index = 0;
    while (index < listeners_pair->value.size())
    {
        if (entry.priority > listeners_pair->value[index].priority)
            break ;
        index++;
    }
    listeners_pair->value.insert(listeners_pair->value.begin() + index, entry);
    if (listeners_pair->value.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(listeners_pair->value.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_game_hooks::append_metadata_unlocked(const ft_game_hook_metadata &metadata) noexcept
{
    this->_catalog_metadata.push_back(metadata);
    if (this->_catalog_metadata.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_catalog_metadata.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_game_hooks::collect_listeners_unlocked(const ft_string &hook_identifier, ft_vector<ft_game_hook_listener_entry> &out_listeners) const noexcept
{
    Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *listeners_pair;
    size_t index;

    listeners_pair = const_cast<ft_game_hooks *>(this)->_listener_catalog.find(hook_identifier);
    if (const_cast<ft_game_hooks *>(this)->_listener_catalog.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(const_cast<ft_game_hooks *>(this)->_listener_catalog.get_error());
        return ;
    }
    if (listeners_pair == const_cast<ft_game_hooks *>(this)->_listener_catalog.end())
    {
        const_cast<ft_game_hooks *>(this)->set_error(FT_ERR_SUCCESSS);
        return ;
    }
    index = 0;
    while (index < listeners_pair->value.size())
    {
        out_listeners.push_back(listeners_pair->value[index]);
        if (out_listeners.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_game_hooks *>(this)->set_error(out_listeners.get_error());
            return ;
        }
        index++;
    }
    const_cast<ft_game_hooks *>(this)->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_game_hooks::collect_metadata_unlocked(ft_vector<ft_game_hook_metadata> &out_metadata) const noexcept
{
    size_t index;

    index = 0;
    while (index < this->_catalog_metadata.size())
    {
        out_metadata.push_back(this->_catalog_metadata[index]);
        if (out_metadata.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_game_hooks *>(this)->set_error(out_metadata.get_error());
            return ;
        }
        index++;
    }
    const_cast<ft_game_hooks *>(this)->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_game_hooks::clone_catalog_from(const ft_game_hooks &other) noexcept
{
    size_t metadata_index;

    metadata_index = 0;
    while (metadata_index < other._catalog_metadata.size())
    {
        const ft_game_hook_metadata &other_metadata = other._catalog_metadata[metadata_index];
        Pair<ft_string, ft_vector<ft_game_hook_listener_entry> > *listeners_pair;
        size_t listener_index;

        listeners_pair = const_cast<ft_game_hooks &>(other)._listener_catalog.find(other_metadata.hook_identifier);
        if (const_cast<ft_game_hooks &>(other)._listener_catalog.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(const_cast<ft_game_hooks &>(other)._listener_catalog.get_error());
            return ;
        }
        if (listeners_pair != const_cast<ft_game_hooks &>(other)._listener_catalog.end())
        {
            listener_index = 0;
            while (listener_index < listeners_pair->value.size())
            {
                if (listeners_pair->value[listener_index].metadata.listener_name == other_metadata.listener_name)
                {
                    ft_game_hook_listener_entry entry;

                    entry.metadata = listeners_pair->value[listener_index].metadata;
                    entry.priority = listeners_pair->value[listener_index].priority;
                    entry.callback = listeners_pair->value[listener_index].callback;
                    if (entry.metadata.hook_identifier.get_error() != FT_ERR_SUCCESSS ||
                        entry.metadata.listener_name.get_error() != FT_ERR_SUCCESSS ||
                        entry.metadata.description.get_error() != FT_ERR_SUCCESSS ||
                        entry.metadata.argument_contract.get_error() != FT_ERR_SUCCESSS)
                    {
                        this->set_error(FT_ERR_INTERNAL);
                        return ;
                    }
                    if (entry.callback.get_error() != FT_ERR_SUCCESSS)
                    {
                        this->set_error(entry.callback.get_error());
                        return ;
                    }
                    this->insert_listener_unlocked(entry);
                    if (this->_error_code != FT_ERR_SUCCESSS)
                        return ;
                    this->append_metadata_unlocked(entry.metadata);
                    if (this->_error_code != FT_ERR_SUCCESSS)
                        return ;
                    break ;
                }
                listener_index++;
            }
        }
        metadata_index++;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_game_hooks::set_on_item_crafted(ft_function<void(ft_character&, ft_item&)> &&callback) noexcept
{
    ft_game_hook_listener_entry entry;
    ft_function<void(ft_character&, ft_item&)> callback_copy;

    entry.metadata = ft_game_hook_metadata();
    entry.priority = 0;
    entry.callback = ft_function<void(ft_game_hook_context&)>();
    callback_copy = ft_function<void(ft_character&, ft_item&)>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_legacy_item_crafted = ft_move(callback);
    if (this->_legacy_item_crafted.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_item_crafted.get_error());
        return ;
    }
    callback_copy = this->_legacy_item_crafted;
    if (callback_copy.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(callback_copy.get_error());
        return ;
    }
    entry.metadata.hook_identifier = ft_game_hook_item_crafted_identifier;
    if (entry.metadata.hook_identifier.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.metadata.hook_identifier.get_error());
        return ;
    }
    entry.metadata.listener_name = "legacy.item_crafted";
    if (entry.metadata.listener_name.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.metadata.listener_name.get_error());
        return ;
    }
    entry.metadata.description = "Legacy callback set via set_on_item_crafted";
    if (entry.metadata.description.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.metadata.description.get_error());
        return ;
    }
    entry.metadata.argument_contract = "ft_character&,ft_item&";
    if (entry.metadata.argument_contract.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.metadata.argument_contract.get_error());
        return ;
    }
    entry.priority = 1000;
    entry.callback = ft_game_hook_make_character_item_adapter(ft_move(callback_copy));
    if (entry.callback.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.callback.get_error());
        return ;
    }
    this->remove_listener_unlocked(entry.metadata.hook_identifier, entry.metadata.listener_name);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->insert_listener_unlocked(entry);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->append_metadata_unlocked(entry.metadata);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_game_hooks::set_on_character_damaged(ft_function<void(ft_character&, int, uint8_t)> &&callback) noexcept
{
    ft_game_hook_listener_entry entry;
    ft_function<void(ft_character&, int, uint8_t)> callback_copy;

    entry.metadata = ft_game_hook_metadata();
    entry.priority = 0;
    entry.callback = ft_function<void(ft_game_hook_context&)>();
    callback_copy = ft_function<void(ft_character&, int, uint8_t)>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_legacy_character_damaged = ft_move(callback);
    if (this->_legacy_character_damaged.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_character_damaged.get_error());
        return ;
    }
    callback_copy = this->_legacy_character_damaged;
    if (callback_copy.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(callback_copy.get_error());
        return ;
    }
    entry.metadata.hook_identifier = ft_game_hook_character_damaged_identifier;
    if (entry.metadata.hook_identifier.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.metadata.hook_identifier.get_error());
        return ;
    }
    entry.metadata.listener_name = "legacy.character_damaged";
    if (entry.metadata.listener_name.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.metadata.listener_name.get_error());
        return ;
    }
    entry.metadata.description = "Legacy callback set via set_on_character_damaged";
    if (entry.metadata.description.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.metadata.description.get_error());
        return ;
    }
    entry.metadata.argument_contract = "ft_character&,int,uint8_t";
    if (entry.metadata.argument_contract.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.metadata.argument_contract.get_error());
        return ;
    }
    entry.priority = 1000;
    entry.callback = ft_game_hook_make_character_damage_adapter(ft_move(callback_copy));
    if (entry.callback.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.callback.get_error());
        return ;
    }
    this->remove_listener_unlocked(entry.metadata.hook_identifier, entry.metadata.listener_name);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->insert_listener_unlocked(entry);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->append_metadata_unlocked(entry.metadata);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_game_hooks::set_on_event_triggered(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept
{
    ft_game_hook_listener_entry entry;
    ft_function<void(ft_world&, ft_event&)> callback_copy;

    entry.metadata = ft_game_hook_metadata();
    entry.priority = 0;
    entry.callback = ft_function<void(ft_game_hook_context&)>();
    callback_copy = ft_function<void(ft_world&, ft_event&)>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_legacy_event_triggered = ft_move(callback);
    if (this->_legacy_event_triggered.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_event_triggered.get_error());
        return ;
    }
    callback_copy = this->_legacy_event_triggered;
    if (callback_copy.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(callback_copy.get_error());
        return ;
    }
    entry.metadata.hook_identifier = ft_game_hook_event_triggered_identifier;
    if (entry.metadata.hook_identifier.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.metadata.hook_identifier.get_error());
        return ;
    }
    entry.metadata.listener_name = "legacy.event_triggered";
    if (entry.metadata.listener_name.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.metadata.listener_name.get_error());
        return ;
    }
    entry.metadata.description = "Legacy callback set via set_on_event_triggered";
    if (entry.metadata.description.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.metadata.description.get_error());
        return ;
    }
    entry.metadata.argument_contract = "ft_world&,ft_event&";
    if (entry.metadata.argument_contract.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.metadata.argument_contract.get_error());
        return ;
    }
    entry.priority = 1000;
    entry.callback = ft_game_hook_make_world_event_adapter(ft_move(callback_copy));
    if (entry.callback.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.callback.get_error());
        return ;
    }
    this->remove_listener_unlocked(entry.metadata.hook_identifier, entry.metadata.listener_name);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->insert_listener_unlocked(entry);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->append_metadata_unlocked(entry.metadata);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_function<void(ft_character&, ft_item&)> ft_game_hooks::get_on_item_crafted() const noexcept
{
    ft_function<void(ft_character&, ft_item&)> callback;

    callback = ft_function<void(ft_character&, ft_item&)>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        return (callback);
    }
    callback = this->_legacy_item_crafted;
    if (callback.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(callback.get_error());
        return (callback);
    }
    const_cast<ft_game_hooks *>(this)->set_error(FT_ERR_SUCCESSS);
    return (callback);
}

ft_function<void(ft_character&, int, uint8_t)> ft_game_hooks::get_on_character_damaged() const noexcept
{
    ft_function<void(ft_character&, int, uint8_t)> callback;

    callback = ft_function<void(ft_character&, int, uint8_t)>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        return (callback);
    }
    callback = this->_legacy_character_damaged;
    if (callback.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(callback.get_error());
        return (callback);
    }
    const_cast<ft_game_hooks *>(this)->set_error(FT_ERR_SUCCESSS);
    return (callback);
}

ft_function<void(ft_world&, ft_event&)> ft_game_hooks::get_on_event_triggered() const noexcept
{
    ft_function<void(ft_world&, ft_event&)> callback;

    callback = ft_function<void(ft_world&, ft_event&)>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        return (callback);
    }
    callback = this->_legacy_event_triggered;
    if (callback.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(callback.get_error());
        return (callback);
    }
    const_cast<ft_game_hooks *>(this)->set_error(FT_ERR_SUCCESSS);
    return (callback);
}

void ft_game_hooks::invoke_hook(const ft_string &hook_identifier, ft_game_hook_context &context) const noexcept
{
    ft_vector<ft_game_hook_listener_entry> listeners;
    size_t index;

    listeners = ft_vector<ft_game_hook_listener_entry>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        return ;
    }
    this->collect_listeners_unlocked(hook_identifier, listeners);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    if (guard.owns_lock())
        guard.unlock();
    index = 0;
    while (index < listeners.size())
    {
        if (listeners[index].callback)
            listeners[index].callback(context);
        index++;
    }
    const_cast<ft_game_hooks *>(this)->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_game_hooks::invoke_on_item_crafted(ft_character &character, ft_item &item) const noexcept
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

void ft_game_hooks::invoke_on_character_damaged(ft_character &character, int damage, uint8_t type) const noexcept
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

void ft_game_hooks::invoke_on_event_triggered(ft_world &world, ft_event &event) const noexcept
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

void ft_game_hooks::register_listener(const ft_game_hook_metadata &metadata, int priority, ft_function<void(ft_game_hook_context&)> &&callback) noexcept
{
    ft_game_hook_listener_entry entry;

    entry.metadata = metadata;
    entry.priority = priority;
    entry.callback = ft_move(callback);
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    if (entry.callback.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(entry.callback.get_error());
        return ;
    }
    this->remove_listener_unlocked(entry.metadata.hook_identifier, entry.metadata.listener_name);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->insert_listener_unlocked(entry);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->append_metadata_unlocked(entry.metadata);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_game_hooks::unregister_listener(const ft_string &hook_identifier, const ft_string &listener_name) noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->remove_listener_unlocked(hook_identifier, listener_name);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_vector<ft_game_hook_metadata> ft_game_hooks::get_catalog_metadata() const noexcept
{
    ft_vector<ft_game_hook_metadata> metadata;

    metadata = ft_vector<ft_game_hook_metadata>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        return (metadata);
    }
    this->collect_metadata_unlocked(metadata);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return (metadata);
    }
    const_cast<ft_game_hooks *>(this)->set_error(FT_ERR_SUCCESSS);
    return (metadata);
}

ft_vector<ft_game_hook_metadata> ft_game_hooks::get_catalog_metadata_for(const ft_string &hook_identifier) const noexcept
{
    ft_vector<ft_game_hook_metadata> metadata;
    ft_vector<ft_game_hook_metadata> full_catalog;
    size_t index;

    metadata = ft_vector<ft_game_hook_metadata>();
    full_catalog = ft_vector<ft_game_hook_metadata>();
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        return (metadata);
    }
    this->collect_metadata_unlocked(full_catalog);
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        return (metadata);
    }
    if (guard.owns_lock())
        guard.unlock();
    index = 0;
    while (index < full_catalog.size())
    {
        if (full_catalog[index].hook_identifier == hook_identifier)
        {
            metadata.push_back(full_catalog[index]);
            if (metadata.get_error() != FT_ERR_SUCCESSS)
            {
                const_cast<ft_game_hooks *>(this)->set_error(metadata.get_error());
                return (metadata);
            }
        }
        index++;
    }
    const_cast<ft_game_hooks *>(this)->set_error(FT_ERR_SUCCESSS);
    return (metadata);
}

void ft_game_hooks::reset() noexcept
{

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_legacy_item_crafted = ft_function<void(ft_character&, ft_item&)>();
    if (this->_legacy_item_crafted.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_item_crafted.get_error());
        return ;
    }
    this->_legacy_character_damaged = ft_function<void(ft_character&, int, uint8_t)>();
    if (this->_legacy_character_damaged.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_character_damaged.get_error());
        return ;
    }
    this->_legacy_event_triggered = ft_function<void(ft_world&, ft_event&)>();
    if (this->_legacy_event_triggered.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_legacy_event_triggered.get_error());
        return ;
    }
    this->_listener_catalog.clear();
    if (this->_listener_catalog.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_listener_catalog.get_error());
        return ;
    }
    this->_catalog_metadata.clear();
    if (this->_catalog_metadata.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_catalog_metadata.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

int ft_game_hooks::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_game_hooks *>(this)->set_error(error_code);
    return (error_code);
}

const char *ft_game_hooks::get_error_str() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_game_hooks *>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_game_hooks *>(this)->set_error(error_code);
    return (ft_strerror(error_code));
}

void ft_game_hooks::set_error(int error) const noexcept
{
    ft_errno = error;
    this->_error_code = error;
    return ;
}

