#include "../PThread/pthread_internal.hpp"
#include "game_scripting_bridge.hpp"
#include "../Template/pair.hpp"
#include "../Template/move.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"
#include <cstdio>
#include <new>

static void trim_whitespace(ft_string &target) noexcept
{
    ft_size_t length;
    ft_size_t end;
    ft_size_t index;
    const char *data;

    length = target.size();
    data = target.c_str();
    index = 0;
    while (index < length
        && (data[index] == ' ' || data[index] == '\t' || data[index] == '\r'))
        index++;
    if (index > 0)
        target.erase(0, index);
    length = target.size();
    if (length == 0)
        return ;
    data = target.c_str();
    end = length;
    while (end > 0
        && (data[end - 1] == ' ' || data[end - 1] == '\t' || data[end - 1] == '\r'))
        end--;
    if (end < length)
        target.erase(end, length - end);
    return ;
}

thread_local int32_t ft_game_script_context::_last_error = FT_ERR_SUCCESS;
thread_local int32_t ft_game_script_bridge::_last_error = FT_ERR_SUCCESS;

int32_t ft_game_script_context::set_error(int32_t error_code) noexcept
{
    ft_game_script_context::_last_error = error_code;
    return (error_code);
}

ft_game_script_context::ft_game_script_context() noexcept
    : _state(ft_nullptr), _world(), _variables(),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_game_script_context::ft_game_script_context(const ft_game_script_context &other) noexcept
    : _state(ft_nullptr), _world(), _variables(),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_game_script_context::ft_game_script_context(copy)",
            "source object is not initialised");
    }
    if (this->initialize(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

ft_game_script_context::ft_game_script_context(ft_game_script_context &&other) noexcept
    : _state(ft_nullptr), _world(), _variables(),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_game_script_context::ft_game_script_context(move)",
            "source object is not initialised");
    }
    if (this->move(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

ft_game_script_context::~ft_game_script_context() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    else
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

int32_t ft_game_script_context::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_game_script_context::initialize", "already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    int32_t variable_error = this->_variables.initialize();
    if (variable_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(variable_error);
        return (variable_error);
    }
    this->_state = ft_nullptr;
    this->_world = ft_sharedptr<ft_world>();
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_game_script_context::initialize(ft_game_state *state,
    const ft_sharedptr<ft_world> &world) noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_game_script_context::initialize", "already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    int32_t variable_error = this->_variables.initialize();
    if (variable_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(variable_error);
        return (variable_error);
    }
    this->_state = state;
    this->_world = world;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_game_script_context::initialize(const ft_game_script_context &other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_game_script_context::initialize", "source object is not initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_game_script_context::initialize", "already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    int32_t variable_error = this->_variables.initialize();
    if (variable_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(variable_error);
        return (variable_error);
    }
    int32_t copy_error = this->_variables.copy_from(other._variables);
    if (copy_error != FT_ERR_SUCCESS)
    {
        (void)this->_variables.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(copy_error);
        return (copy_error);
    }
    this->_state = other._state;
    this->_world = other._world;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(other.get_error());
    return (FT_ERR_SUCCESS);
}

int32_t ft_game_script_context::move(ft_game_script_context &other) noexcept
{
    return (this->initialize(static_cast<const ft_game_script_context &>(other)));
}

int32_t ft_game_script_context::destroy() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_state = ft_nullptr;
    this->_world = ft_sharedptr<ft_world>();
    int32_t destroy_error = this->_variables.destroy();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(destroy_error);
    return (destroy_error);
}

ft_game_state *ft_game_script_context::get_state() const noexcept
{
    return (this->_state);
}

const ft_sharedptr<ft_world> &ft_game_script_context::get_world() const noexcept
{
    return (this->_world);
}

void ft_game_script_context::set_state(ft_game_state *state) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_game_script_context::set_state");
    this->_state = state;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_game_script_context::set_world(const ft_sharedptr<ft_world> &world) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_game_script_context::set_world");
    this->_world = world;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_game_script_context::set_variable(const ft_string &key, const ft_string &value) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_game_script_context::set_variable");
    if (this->_state)
    {
        this->_state->set_variable(key, value);
        this->set_error(this->_state->get_error());
        return ;
    }
    Pair<ft_string, ft_string> *entry;

    entry = this->_variables.find(key);
    if (entry != this->_variables.end())
    {
        entry->value = value;
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    this->_variables.insert(key, value);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

const ft_string *ft_game_script_context::get_variable(const ft_string &key) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_game_script_context::get_variable");
    if (this->_state)
    {
        const ft_string *value;

        value = this->_state->get_variable(key);
        this->set_error(this->_state->get_error());
        return (value);
    }
    const Pair<ft_string, ft_string> *entry;

    entry = this->_variables.find(key);
    if (entry == this->_variables.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (&entry->value);
}

void ft_game_script_context::remove_variable(const ft_string &key) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_game_script_context::remove_variable");
    if (this->_state)
    {
        this->_state->remove_variable(key);
        this->set_error(this->_state->get_error());
        return ;
    }
    this->_variables.remove(key);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_game_script_context::clear_variables() noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_game_script_context::clear_variables");
    if (this->_state)
    {
        this->_state->clear_variables();
        this->set_error(this->_state->get_error());
        return ;
    }
    this->_variables.clear();
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t ft_game_script_context::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_game_script_context::get_error");
    return (ft_game_script_context::_last_error);
}

const char *ft_game_script_context::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_game_script_context::get_error_str");
    return (ft_strerror(ft_game_script_context::_last_error));
}

int32_t ft_game_script_bridge::set_error(int32_t error_code) noexcept
{
    ft_game_script_bridge::_last_error = error_code;
    return (error_code);
}

ft_bool ft_game_script_bridge::is_supported_language(const ft_string &language) noexcept
{
    ft_string normalized;
    char *data;

    normalized = language;
    data = normalized.data();
    if (data)
        ft_to_lower(data);
    if (normalized == "lua")
        return (FT_TRUE);
    if (normalized == "python")
        return (FT_TRUE);
    return (FT_FALSE);
}

ft_game_script_bridge::ft_game_script_bridge() noexcept
    : _world(), _callbacks(), _language(), _max_operations(32),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _mutex(ft_nullptr)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_game_script_bridge::ft_game_script_bridge(const ft_game_script_bridge &other) noexcept
    : _world(), _callbacks(), _language(), _max_operations(32),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _mutex(ft_nullptr)
{
    this->set_error(FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_game_script_bridge::ft_game_script_bridge(copy)",
            "source object is not initialised");
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    if (this->initialize(other._world, other._language.c_str()) != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    this->_max_operations = other._max_operations;
    this->_callbacks = other._callbacks;
    this->set_error(other.get_error());
    return ;
}

ft_game_script_bridge::ft_game_script_bridge(ft_game_script_bridge &&other) noexcept
    : _world(), _callbacks(), _language(), _max_operations(32),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _mutex(ft_nullptr)
{
    this->set_error(FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_game_script_bridge::ft_game_script_bridge(move)",
            "source object is not initialised");
    }
    if (this->move(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

int32_t ft_game_script_bridge::initialize() noexcept
{
    return (this->initialize(ft_sharedptr<ft_world>(), "lua"));
}

int32_t ft_game_script_bridge::initialize(const ft_sharedptr<ft_world> &world,
    const char *language) noexcept
{
    int32_t map_error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state,
            "ft_game_script_bridge::initialize",
            "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    map_error = this->_callbacks.initialize();
    if (map_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(map_error);
        return (map_error);
    }
    this->_world = world;
    if (language)
        this->_language = language;
    else
        this->_language = "lua";
    if (ft_game_script_bridge::is_supported_language(this->_language) == FT_FALSE)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

ft_game_script_bridge::~ft_game_script_bridge() noexcept
{
    (void)this->destroy();
    return ;
}

int32_t ft_game_script_bridge::destroy() noexcept
{
    int32_t first_error;
    int32_t current_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    first_error = FT_ERR_SUCCESS;
    current_error = this->disable_thread_safety();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_callbacks.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    this->_world = ft_sharedptr<ft_world>();
    this->_language.clear();
    this->_max_operations = 32;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(first_error);
    return (first_error);
}

int32_t ft_game_script_bridge::move(ft_game_script_bridge &other) noexcept
{
    int32_t destroy_error;
    int32_t initialize_error;
    Pair<ft_string, ft_function<int32_t(ft_game_script_context &,
        const ft_vector<ft_string> &)> > *entry;
    Pair<ft_string, ft_function<int32_t(ft_game_script_context &,
        const ft_vector<ft_string> &)> > *entry_end;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_game_script_bridge::move", "source object is not initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    destroy_error = this->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize(other._world, other._language.c_str());
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_max_operations = other._max_operations;
    entry = other._callbacks.end() - other._callbacks.size();
    entry_end = other._callbacks.end();
    while (entry != entry_end)
    {
        initialize_error = this->register_function(entry->key, entry->value);
        if (initialize_error != FT_ERR_SUCCESS)
        {
            (void)this->destroy();
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            this->set_error(initialize_error);
            return (initialize_error);
        }
        entry++;
    }
    (void)other.destroy();
    this->set_error(other.get_error());
    return (FT_ERR_SUCCESS);
}

int32_t ft_game_script_bridge::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void ft_game_script_bridge::unlock_internal(ft_bool lock_acquired) const noexcept
{

    if (lock_acquired == FT_FALSE)
        return ;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void ft_game_script_bridge::set_language(const char *language) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }

    if (!language)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        return ;
    }
    ft_string candidate;
    if (candidate.initialize(language) != FT_ERR_SUCCESS)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    if (ft_game_script_bridge::is_supported_language(candidate) == FT_FALSE)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->_language = candidate;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_string &ft_game_script_bridge::get_language() const noexcept
{
    return (this->_language);
}

void ft_game_script_bridge::set_max_operations(int32_t limit) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }

    if (limit < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->_max_operations = limit;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int32_t ft_game_script_bridge::get_max_operations() const noexcept
{
    return (this->_max_operations);
}

ft_size_t ft_game_script_bridge::get_callback_count() const noexcept
{
    return (this->_callbacks.size());
}

int32_t ft_game_script_bridge::register_function(const ft_string &name, const ft_function<int32_t(ft_game_script_context &, const ft_vector<ft_string> &)> &callback) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    Pair<ft_string, ft_function<int32_t(ft_game_script_context &, const ft_vector<ft_string> &)> > *entry;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }

    if (name.empty() || !callback)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    entry = this->_callbacks.find(name);
    if (entry != this->_callbacks.end())
    {
        entry->value = callback;
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_callbacks.insert(name, callback);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t ft_game_script_bridge::remove_function(const ft_string &name) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }

    this->_callbacks.remove(name);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void ft_game_script_bridge::tokenize_line(const ft_string &line, ft_vector<ft_string> &tokens) const noexcept
{
    const char *data;
    ft_size_t length;
    ft_size_t index;

    tokens.clear();
    data = line.c_str();
    length = line.size();
    index = 0;
    while (index < length)
    {
        while (index < length
            && (data[index] == ' ' || data[index] == '\t'))
            index++;
        if (index >= length)
            break ;
        ft_size_t start;
        ft_size_t end;

        start = index;
        while (index < length
            && data[index] != ' '
            && data[index] != '\t')
            index++;
        end = index;
        ft_string token = line.substr(start, end - start);
        tokens.push_back(ft_move(token));
    }
    return ;
}

int32_t ft_game_script_bridge::handle_set(ft_game_script_context &context, const ft_vector<ft_string> &tokens) noexcept
{
    ft_string value;
    ft_size_t index;
    ft_size_t count;

    if (tokens.size() < 3)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    value = tokens[2];
    count = tokens.size();
    index = 3;
    while (index < count)
    {
        value.append(" ");
        value.append(tokens[index]);
        index++;
    }
    context.set_variable(tokens[1], value);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_game_script_bridge::handle_unset(ft_game_script_context &context, const ft_vector<ft_string> &tokens) noexcept
{
    if (tokens.size() < 2)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    context.remove_variable(tokens[1]);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_game_script_bridge::handle_call(ft_game_script_context &context, const ft_vector<ft_string> &tokens) noexcept
{
    ft_vector<ft_string> arguments;
    Pair<ft_string, ft_function<int32_t(ft_game_script_context &, const ft_vector<ft_string> &)> > *entry;
    ft_size_t count;
    ft_size_t index;
    int32_t result;

    if (tokens.size() < 2)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    entry = this->_callbacks.find(tokens[1]);
    if (entry == this->_callbacks.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    if (!entry->value)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    count = tokens.size();
    index = 2;
    while (index < count)
    {
        ft_string argument = tokens[index];
        arguments.push_back(ft_move(argument));
        index++;
    }
    result = entry->value(context, arguments);
    if (result != FT_ERR_SUCCESS)
    {
        this->set_error(result);
        return (result);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_game_script_bridge::execute_line(ft_game_script_context &context, const ft_string &line) noexcept
{
    ft_vector<ft_string> tokens;
    ft_string command;
    char *command_data;

    this->tokenize_line(line, tokens);
    if (tokens.empty())
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    command = tokens[0];
    command_data = command.data();
    if (command_data)
        ft_to_lower(command_data);
    if (command == "call")
        return (this->handle_call(context, tokens));
    if (command == "set")
        return (this->handle_set(context, tokens));
    if (command == "unset")
        return (this->handle_unset(context, tokens));
    this->set_error(FT_ERR_INVALID_ARGUMENT);
    return (FT_ERR_INVALID_ARGUMENT);
}

int32_t ft_game_script_bridge::execute(const ft_string &script, ft_game_state &state) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_game_script_context context;
    int32_t context_init_error;
    const char *data;
    ft_size_t length;
    ft_size_t start;
    int32_t operations;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    context_init_error = context.initialize(&state, this->_world);
    if (context_init_error != FT_ERR_SUCCESS)
    {
        this->set_error(context_init_error);
        this->unlock_internal(lock_acquired);
        return (context_init_error);
    }
    if (ft_game_script_bridge::is_supported_language(this->_language) == FT_FALSE)
    {
        this->set_error(FT_ERR_CONFIGURATION);
        this->unlock_internal(lock_acquired);
        return (FT_ERR_CONFIGURATION);
    }
    data = script.c_str();
    length = script.size();
    start = 0;
    operations = 0;
    while (start <= length)
    {
        ft_size_t index;
        ft_size_t count;
        ft_string line;
        const char *line_data;

        if (start >= length)
            break ;
        index = start;
        while (index < length
            && data[index] != '\n'
            && data[index] != '\r')
            index++;
        count = index - start;
        line = script.substr(start, count);
        trim_whitespace(line);
        if (!line.empty())
        {
            line_data = line.c_str();
            if (!(line.size() >= 2 && line_data[0] == '-' && line_data[1] == '-')
                && !(line_data[0] == '#')
                && !(line_data[0] == ';'))
            {
                operations++;
                if (this->_max_operations > 0 && operations > this->_max_operations)
                {
                    this->set_error(FT_ERR_INVALID_OPERATION);
                    this->unlock_internal(lock_acquired);
                    return (FT_ERR_INVALID_OPERATION);
                }
                int32_t result = this->execute_line(context, line);
                if (result != FT_ERR_SUCCESS)
                {
                    this->set_error(result);
                    this->unlock_internal(lock_acquired);
                    return (result);
                }
            }
        }
        if (index >= length)
            break ;
        while (index < length
            && (data[index] == '\n' || data[index] == '\r'))
            index++;
        start = index;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t ft_game_script_bridge::check_sandbox_capabilities(const ft_string &script, ft_vector<ft_string> &violations) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    const char *data;
    ft_size_t length;
    ft_size_t start;
    int32_t operations;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    violations.clear();
    data = script.c_str();
    length = script.size();
    start = 0;
    operations = 0;
    while (start <= length)
    {
        ft_size_t index;
        ft_size_t count;
        ft_string line;
        const char *line_data;

        if (start >= length)
            break ;
        index = start;
        while (index < length
            && data[index] != '\n'
            && data[index] != '\r')
            index++;
        count = index - start;
        line = script.substr(start, count);
        trim_whitespace(line);
        if (!line.empty())
        {
            line_data = line.c_str();
            if (!(line.size() >= 2 && line_data[0] == '-' && line_data[1] == '-')
                && !(line_data[0] == '#')
                && !(line_data[0] == ';'))
            {
                ft_vector<ft_string> tokens;
                ft_string command_original;
                ft_string command_normalized;
                char *command_data;

                operations++;
                this->tokenize_line(line, tokens);
                if (!tokens.empty())
                {
                    command_original = tokens[0];
                    command_normalized = command_original;
                    command_data = command_normalized.data();
                    if (command_data)
                        ft_to_lower(command_data);
                    if (!(command_normalized == "call"
                        || command_normalized == "set"
                        || command_normalized == "unset"))
                    {
                        ft_string violation;
                        if (violation.initialize("unsupported command: ") != FT_ERR_SUCCESS)
                        {
                            this->set_error(FT_ERR_NO_MEMORY);
                            return (FT_ERR_NO_MEMORY);
                        }
                        violation.append(command_original);
                        violations.push_back(ft_move(violation));
                    }
                }
            }
        }
        if (index >= length)
            break ;
        while (index < length
            && (data[index] == '\n' || data[index] == '\r'))
            index++;
        start = index;
    }
    if (this->_max_operations > 0 && operations > this->_max_operations)
    {
        ft_string violation;
        if (violation.initialize("operation budget exceeded: ") != FT_ERR_SUCCESS)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            this->unlock_internal(lock_acquired);
            return (FT_ERR_NO_MEMORY);
        }
        ft_string operation_text;
        ft_string limit_text;
        {
            char operation_buffer[64];

            std::snprintf(operation_buffer, sizeof(operation_buffer), "%d", operations);
            operation_text = operation_buffer;
        }
        violation.append(operation_text);
        violation.append(" > ");
        {
            char limit_buffer[64];

            std::snprintf(limit_buffer, sizeof(limit_buffer), "%d", this->_max_operations);
            limit_text = limit_buffer;
        }
        violation.append(limit_text);
        violations.push_back(ft_move(violation));
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t ft_game_script_bridge::validate_dry_run(const ft_string &script, ft_vector<ft_string> &warnings) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    const char *data;
    ft_size_t length;
    ft_size_t start;
    int32_t operations;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    warnings.clear();
    data = script.c_str();
    length = script.size();
    start = 0;
    operations = 0;
    while (start <= length)
    {
        ft_size_t index;
        ft_size_t count;
        ft_string line;
        const char *line_data;

        if (start >= length)
            break ;
        index = start;
        while (index < length
            && data[index] != '\n'
            && data[index] != '\r')
            index++;
        count = index - start;
        line = script.substr(start, count);
        trim_whitespace(line);
        if (!line.empty())
        {
            line_data = line.c_str();
            if (!(line.size() >= 2 && line_data[0] == '-' && line_data[1] == '-')
                && !(line_data[0] == '#')
                && !(line_data[0] == ';'))
            {
                ft_vector<ft_string> tokens;
                ft_string command_original;
                ft_string command_normalized;
                char *command_data;

                operations++;
                this->tokenize_line(line, tokens);
                if (!tokens.empty())
                {
                    command_original = tokens[0];
                    command_normalized = command_original;
                    command_data = command_normalized.data();
                    if (command_data)
                        ft_to_lower(command_data);
                    if (command_normalized == "call")
                    {
                        if (tokens.size() < 2)
                        {
                            ft_string warning;
                            if (warning.initialize("call missing target") != FT_ERR_SUCCESS)
                                return (FT_ERR_NO_MEMORY);
                            warnings.push_back(ft_move(warning));
                        }
                        else
                        {
                            ft_string callback_name;
                            Pair<ft_string, ft_function<int32_t(ft_game_script_context &, const ft_vector<ft_string> &)> > *entry;

                            callback_name = tokens[1];
                            entry = this->_callbacks.find(callback_name);
                            if (entry == this->_callbacks.end())
                            {
                                ft_string warning;
                                if (warning.initialize("unregistered callback: ") != FT_ERR_SUCCESS)
                                    return (FT_ERR_NO_MEMORY);
                                warning.append(callback_name);
                                warnings.push_back(ft_move(warning));
                            }
                            else if (!entry->value)
                            {
                                ft_string warning;
                                if (warning.initialize("callback missing target: ") != FT_ERR_SUCCESS)
                                    return (FT_ERR_NO_MEMORY);
                                warning.append(callback_name);
                                warnings.push_back(ft_move(warning));
                            }
                            else if (tokens.size() < 3)
                            {
                                ft_string warning;
                                if (warning.initialize("call missing arguments: ")
                                        != FT_ERR_SUCCESS)
                                    return (FT_ERR_NO_MEMORY);
                                warning.append(callback_name);
                                warnings.push_back(ft_move(warning));
                            }
                        }
                    }
                    else if (command_normalized == "set")
                    {
                        if (tokens.size() < 2)
                        {
                            ft_string warning;
                            if (warning.initialize("set missing key") != FT_ERR_SUCCESS)
                                return (FT_ERR_NO_MEMORY);
                            warnings.push_back(ft_move(warning));
                        }
                        else if (tokens.size() < 3)
                        {
                            ft_string warning;
                            if (warning.initialize("set missing value for key: ") != FT_ERR_SUCCESS)
                                return (FT_ERR_NO_MEMORY);
                            ft_string missing_key;
                            missing_key = tokens[1];
                            warning.append(missing_key);
                            warnings.push_back(ft_move(warning));
                        }
                    }
                    else if (command_normalized == "unset")
                    {
                        if (tokens.size() < 2)
                        {
                            ft_string warning;
                            if (warning.initialize("unset missing key") != FT_ERR_SUCCESS)
                                return (FT_ERR_NO_MEMORY);
                            warnings.push_back(ft_move(warning));
                        }
                    }
                }
            }
        }
        if (index >= length)
            break ;
        while (index < length
            && (data[index] == '\n' || data[index] == '\r'))
            index++;
        start = index;
    }
    if (this->_max_operations > 0 && operations > this->_max_operations)
    {
        ft_string warning;
        if (warning.initialize("operation budget exceeded: ") != FT_ERR_SUCCESS)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            this->unlock_internal(lock_acquired);
            return (FT_ERR_NO_MEMORY);
        }
        ft_string operation_text;
        ft_string limit_text;
        {
            char operation_buffer[64];

            std::snprintf(operation_buffer, sizeof(operation_buffer), "%d", operations);
            operation_text = operation_buffer;
        }
        warning.append(operation_text);
        warning.append(" > ");
        {
            char limit_buffer[64];

            std::snprintf(limit_buffer, sizeof(limit_buffer), "%d", this->_max_operations);
            limit_text = limit_buffer;
        }
        warning.append(limit_text);
        warnings.push_back(ft_move(warning));
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t ft_game_script_bridge::inspect_bytecode_budget(const ft_string &script, int32_t &required_operations) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    const char *data;
    ft_size_t length;
    ft_size_t start;
    int32_t operations;

    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    data = script.c_str();
    length = script.size();
    start = 0;
    operations = 0;
    while (start <= length)
    {
        ft_size_t index;
        ft_size_t count;
        ft_string line;
        const char *line_data;

        if (start >= length)
            break ;
        index = start;
        while (index < length
            && data[index] != '\n'
            && data[index] != '\r')
            index++;
        count = index - start;
        line = script.substr(start, count);
        trim_whitespace(line);
        if (!line.empty())
        {
            line_data = line.c_str();
            if (!(line.size() >= 2 && line_data[0] == '-' && line_data[1] == '-')
                && !(line_data[0] == '#')
                && !(line_data[0] == ';'))
            {
                ft_vector<ft_string> tokens;
                ft_string command_original;
                ft_string command_normalized;
                char *command_data;

                this->tokenize_line(line, tokens);
                if (!tokens.empty())
                {
                    command_original = tokens[0];
                    command_normalized = command_original;
                    command_data = command_normalized.data();
                    if (command_data)
                        ft_to_lower(command_data);
                    if (command_normalized == "call"
                        || command_normalized == "set"
                        || command_normalized == "unset")
                    {
                        operations++;
                    }
                    else
                    {
                        required_operations = operations;
                        this->set_error(FT_ERR_INVALID_ARGUMENT);
                        this->unlock_internal(lock_acquired);
                        return (FT_ERR_INVALID_ARGUMENT);
                    }
                }
            }
        }
        if (index >= length)
            break ;
        while (index < length
            && (data[index] == '\n' || data[index] == '\r'))
            index++;
        start = index;
    }
    required_operations = operations;
    if (this->_max_operations > 0 && operations > this->_max_operations)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        return (FT_ERR_INVALID_OPERATION);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t ft_game_script_bridge::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

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

int32_t ft_game_script_bridge::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

ft_bool ft_game_script_bridge::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_game_script_bridge::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_game_script_bridge::get_error");
    return (ft_game_script_bridge::_last_error);
}

const char *ft_game_script_bridge::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_game_script_bridge::get_error_str");
    return (ft_strerror(ft_game_script_bridge::_last_error));
}
