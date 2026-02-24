#include "game_scripting_bridge.hpp"
#include "../Template/pair.hpp"
#include "../Template/move.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdio>
#include <new>

static void trim_whitespace(ft_string &target) noexcept
{
    size_t length;
    size_t end;
    size_t index;
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

void ft_game_script_context::set_error(int error) const noexcept
{
    this->_error_code = error;
    return ;
}

ft_game_script_context::ft_game_script_context() noexcept
    : _state(ft_nullptr), _world(), _variables(), _error_code(FT_ERR_SUCCESS)
{
    return ;
}

ft_game_script_context::~ft_game_script_context() noexcept
{
    return ;
}

int ft_game_script_context::initialize() noexcept
{
    this->_state = ft_nullptr;
    this->_world = ft_sharedptr<ft_world>();
    this->_variables.clear();
    this->_error_code = FT_ERR_SUCCESS;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_game_script_context::initialize(ft_game_state *state,
    const ft_sharedptr<ft_world> &world) noexcept
{
    this->_state = state;
    this->_world = world;
    this->_variables.clear();
    this->_error_code = FT_ERR_SUCCESS;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_game_script_context::initialize(const ft_game_script_context &other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESS);
    this->_state = other._state;
    this->_world = other._world;
    this->_variables = other._variables;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    return (FT_ERR_SUCCESS);
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
    this->_state = state;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_game_script_context::set_world(const ft_sharedptr<ft_world> &world) noexcept
{
    this->_world = world;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_game_script_context::set_variable(const ft_string &key, const ft_string &value) noexcept
{
    if (this->_state)
    {
        this->_state->set_variable(key, value);
        this->set_error(this->_state->get_error());
        return ;
    }
    Pair<ft_string, ft_string> *entry;

    entry = this->_variables.find(key);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    if (entry != this->_variables.end())
    {
        entry->value = value;
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    this->_variables.insert(key, value);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

const ft_string *ft_game_script_context::get_variable(const ft_string &key) const noexcept
{
    if (this->_state)
    {
        const ft_string *value;

        value = this->_state->get_variable(key);
        this->set_error(this->_state->get_error());
        return (value);
    }
    const Pair<ft_string, ft_string> *entry;

    entry = this->_variables.find(key);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (ft_nullptr);
    }
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
    if (this->_state)
    {
        this->_state->remove_variable(key);
        this->set_error(this->_state->get_error());
        return ;
    }
    this->_variables.remove(key);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_game_script_context::clear_variables() noexcept
{
    if (this->_state)
    {
        this->_state->clear_variables();
        this->set_error(this->_state->get_error());
        return ;
    }
    this->_variables.clear();
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_game_script_context::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_game_script_context::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void ft_game_script_bridge::set_error(int error) const noexcept
{
    this->_error_code = error;
    return ;
}

bool ft_game_script_bridge::is_supported_language(const ft_string &language) noexcept
{
    ft_string normalized;
    char *data;

    normalized = language;
    data = normalized.data();
    if (data)
        ft_to_lower(data);
    if (normalized == "lua")
        return (true);
    if (normalized == "python")
        return (true);
    return (false);
}

ft_game_script_bridge::ft_game_script_bridge() noexcept
    : _world(), _callbacks(), _language(), _max_operations(32),
      _error_code(FT_ERR_SUCCESS), _mutex(ft_nullptr)
{
    this->_language = "lua";
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_game_script_bridge::initialize(const ft_sharedptr<ft_world> &world,
    const char *language) noexcept
{
    this->_world = world;
    if (language)
        this->_language = language;
    else
        this->_language = "lua";
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (ft_game_script_bridge::is_supported_language(this->_language) == false)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

ft_game_script_bridge::~ft_game_script_bridge() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

int ft_game_script_bridge::lock_internal(bool *lock_acquired) const noexcept
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

void ft_game_script_bridge::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return ;
    if (this->_mutex == ft_nullptr)
        return ;
    (void)this->_mutex->unlock();
    return ;
}

void ft_game_script_bridge::set_language(const char *language) noexcept
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

    if (!language)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        return ;
    }
    ft_string candidate(language);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    if (ft_game_script_bridge::is_supported_language(candidate) == false)
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

void ft_game_script_bridge::set_max_operations(int limit) noexcept
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

int ft_game_script_bridge::get_max_operations() const noexcept
{
    return (this->_max_operations);
}

size_t ft_game_script_bridge::get_callback_count() const noexcept
{
    return (this->_callbacks.size());
}

int ft_game_script_bridge::register_function(const ft_string &name, const ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> &callback) noexcept
{
    bool lock_acquired;
    int lock_error;
    Pair<ft_string, ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> > *entry;

    lock_acquired = false;
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
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        this->unlock_internal(lock_acquired);
        return (FT_ERR_SUCCESS);
    }
    if (entry != this->_callbacks.end())
    {
        entry->value = callback;
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_callbacks.insert(name, callback);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        this->unlock_internal(lock_acquired);
        return (FT_ERR_SUCCESS);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_game_script_bridge::remove_function(const ft_string &name) noexcept
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

    this->_callbacks.remove(name);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        this->unlock_internal(lock_acquired);
        return (FT_ERR_SUCCESS);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void ft_game_script_bridge::tokenize_line(const ft_string &line, ft_vector<ft_string> &tokens) const noexcept
{
    const char *data;
    size_t length;
    size_t index;

    tokens.clear();
    if (false)
        return ;
    data = line.c_str();
    length = line.size();
    index = 0;
    while (index < length)
    {
        while (index < length
            && (data[index] == ' ' || data[index] == '\t'))
            index++;
        if (index >= length)
            break;
        size_t start;
        size_t end;

        start = index;
        while (index < length
            && data[index] != ' '
            && data[index] != '\t')
            index++;
        end = index;
        ft_string token = line.substr(start, end - start);
        if (false)
        {
            tokens.clear();
            return ;
        }
        tokens.push_back(ft_move(token));
        if (false)
            return ;
    }
    return ;
}

int ft_game_script_bridge::handle_set(ft_game_script_context &context, const ft_vector<ft_string> &tokens) noexcept
{
    ft_string value;
    size_t index;
    size_t count;

    if (tokens.size() < 3)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    value = tokens[2];
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    count = tokens.size();
    index = 3;
    while (index < count)
    {
        value.append(" ");
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        value.append(tokens[index]);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        index++;
    }
    context.set_variable(tokens[1], value);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_game_script_bridge::handle_unset(ft_game_script_context &context, const ft_vector<ft_string> &tokens) noexcept
{
    if (tokens.size() < 2)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    context.remove_variable(tokens[1]);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_game_script_bridge::handle_call(ft_game_script_context &context, const ft_vector<ft_string> &tokens) noexcept
{
    ft_vector<ft_string> arguments;
    Pair<ft_string, ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> > *entry;
    size_t count;
    size_t index;
    int result;

    if (tokens.size() < 2)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    entry = this->_callbacks.find(tokens[1]);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
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
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        arguments.push_back(ft_move(argument));
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        index++;
    }
    result = entry->value(context, arguments);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (result != FT_ERR_SUCCESS)
    {
        this->set_error(result);
        return (result);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_game_script_bridge::execute_line(ft_game_script_context &context, const ft_string &line) noexcept
{
    ft_vector<ft_string> tokens;
    ft_string command;
    char *command_data;

    this->tokenize_line(line, tokens);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (tokens.empty())
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    command = tokens[0];
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
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

int ft_game_script_bridge::execute(const ft_string &script, ft_game_state &state) noexcept
{
    bool lock_acquired;
    int lock_error;
    ft_game_script_context context;
    int context_init_error;
    const char *data;
    size_t length;
    size_t start;
    int operations;

    lock_acquired = false;
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
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (ft_game_script_bridge::is_supported_language(this->_language) == false)
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
        size_t index;
        size_t count;
        ft_string line;
        const char *line_data;

        if (start >= length)
            break;
        index = start;
        while (index < length
            && data[index] != '\n'
            && data[index] != '\r')
            index++;
        count = index - start;
        line = script.substr(start, count);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        trim_whitespace(line);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
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
                int result = this->execute_line(context, line);
                if (result != FT_ERR_SUCCESS)
                {
                    this->set_error(result);
                    this->unlock_internal(lock_acquired);
                    return (result);
                }
            }
        }
        if (index >= length)
            break;
        while (index < length
            && (data[index] == '\n' || data[index] == '\r'))
            index++;
        start = index;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_game_script_bridge::check_sandbox_capabilities(const ft_string &script, ft_vector<ft_string> &violations) noexcept
{
    bool lock_acquired;
    int lock_error;
    const char *data;
    size_t length;
    size_t start;
    int operations;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    violations.clear();
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    data = script.c_str();
    length = script.size();
    start = 0;
    operations = 0;
    while (start <= length)
    {
        size_t index;
        size_t count;
        ft_string line;
        const char *line_data;

        if (start >= length)
            break;
        index = start;
        while (index < length
            && data[index] != '\n'
            && data[index] != '\r')
            index++;
        count = index - start;
        line = script.substr(start, count);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        trim_whitespace(line);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
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
                if (false)
                {
                    this->set_error(FT_ERR_SUCCESS);
                    return (FT_ERR_SUCCESS);
                }
                if (!tokens.empty())
                {
                    command_original = tokens[0];
                    if (false)
                    {
                        this->set_error(FT_ERR_SUCCESS);
                        return (FT_ERR_SUCCESS);
                    }
                    command_normalized = command_original;
                    if (false)
                    {
                        this->set_error(FT_ERR_SUCCESS);
                        return (FT_ERR_SUCCESS);
                    }
                    command_data = command_normalized.data();
                    if (command_data)
                        ft_to_lower(command_data);
                    if (!(command_normalized == "call"
                        || command_normalized == "set"
                        || command_normalized == "unset"))
                    {
                        ft_string violation("unsupported command: ");
                        if (false)
                        {
                            this->set_error(FT_ERR_SUCCESS);
                            return (FT_ERR_SUCCESS);
                        }
                        violation.append(command_original);
                        if (false)
                        {
                            this->set_error(FT_ERR_SUCCESS);
                            return (FT_ERR_SUCCESS);
                        }
                        violations.push_back(ft_move(violation));
                        if (false)
                        {
                            this->set_error(FT_ERR_SUCCESS);
                            return (FT_ERR_SUCCESS);
                        }
                    }
                }
            }
        }
        if (index >= length)
            break;
        while (index < length
            && (data[index] == '\n' || data[index] == '\r'))
            index++;
        start = index;
    }
    if (this->_max_operations > 0 && operations > this->_max_operations)
    {
        ft_string violation("operation budget exceeded: ");
        ft_string operation_text;
        ft_string limit_text;

        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        {
            char operation_buffer[64];

            std::snprintf(operation_buffer, sizeof(operation_buffer), "%d", operations);
            operation_text = operation_buffer;
        }
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        violation.append(operation_text);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        violation.append(" > ");
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        {
            char limit_buffer[64];

            std::snprintf(limit_buffer, sizeof(limit_buffer), "%d", this->_max_operations);
            limit_text = limit_buffer;
        }
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        violation.append(limit_text);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        violations.push_back(ft_move(violation));
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_game_script_bridge::validate_dry_run(const ft_string &script, ft_vector<ft_string> &warnings) noexcept
{
    bool lock_acquired;
    int lock_error;
    const char *data;
    size_t length;
    size_t start;
    int operations;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    warnings.clear();
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    data = script.c_str();
    length = script.size();
    start = 0;
    operations = 0;
    while (start <= length)
    {
        size_t index;
        size_t count;
        ft_string line;
        const char *line_data;

        if (start >= length)
            break;
        index = start;
        while (index < length
            && data[index] != '\n'
            && data[index] != '\r')
            index++;
        count = index - start;
        line = script.substr(start, count);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        trim_whitespace(line);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
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
                if (false)
                {
                    this->set_error(FT_ERR_SUCCESS);
                    return (FT_ERR_SUCCESS);
                }
                if (!tokens.empty())
                {
                    command_original = tokens[0];
                    if (false)
                    {
                        this->set_error(FT_ERR_SUCCESS);
                        return (FT_ERR_SUCCESS);
                    }
                    command_normalized = command_original;
                    if (false)
                    {
                        this->set_error(FT_ERR_SUCCESS);
                        return (FT_ERR_SUCCESS);
                    }
                    command_data = command_normalized.data();
                    if (command_data)
                        ft_to_lower(command_data);
                    if (command_normalized == "call")
                    {
                        if (tokens.size() < 2)
                        {
                            ft_string warning("call missing target");

                            if (false)
                            {
                                this->set_error(FT_ERR_SUCCESS);
                                return (FT_ERR_SUCCESS);
                            }
                            warnings.push_back(ft_move(warning));
                            if (false)
                            {
                                this->set_error(FT_ERR_SUCCESS);
                                return (FT_ERR_SUCCESS);
                            }
                        }
                        else
                        {
                            ft_string callback_name;
                            Pair<ft_string, ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> > *entry;

                            callback_name = tokens[1];
                            if (false)
                            {
                                this->set_error(FT_ERR_SUCCESS);
                                return (FT_ERR_SUCCESS);
                            }
                            entry = this->_callbacks.find(callback_name);
                            if (false)
                            {
                                this->set_error(FT_ERR_SUCCESS);
                                return (FT_ERR_SUCCESS);
                            }
                            if (entry == this->_callbacks.end())
                            {
                                ft_string warning("unregistered callback: ");

                                if (false)
                                {
                                    this->set_error(FT_ERR_SUCCESS);
                                    return (FT_ERR_SUCCESS);
                                }
                                warning.append(callback_name);
                                if (false)
                                {
                                    this->set_error(FT_ERR_SUCCESS);
                                    return (FT_ERR_SUCCESS);
                                }
                                warnings.push_back(ft_move(warning));
                                if (false)
                                {
                                    this->set_error(FT_ERR_SUCCESS);
                                    return (FT_ERR_SUCCESS);
                                }
                            }
                            else if (!entry->value)
                            {
                                ft_string warning("callback missing target: ");

                                if (false)
                                {
                                    this->set_error(FT_ERR_SUCCESS);
                                    return (FT_ERR_SUCCESS);
                                }
                                warning.append(callback_name);
                                if (false)
                                {
                                    this->set_error(FT_ERR_SUCCESS);
                                    return (FT_ERR_SUCCESS);
                                }
                                warnings.push_back(ft_move(warning));
                                if (false)
                                {
                                    this->set_error(FT_ERR_SUCCESS);
                                    return (FT_ERR_SUCCESS);
                                }
                            }
                            else if (tokens.size() < 3)
                            {
                                ft_string warning("call missing arguments: ");

                                if (false)
                                {
                                    this->set_error(FT_ERR_SUCCESS);
                                    return (FT_ERR_SUCCESS);
                                }
                                warning.append(callback_name);
                                if (false)
                                {
                                    this->set_error(FT_ERR_SUCCESS);
                                    return (FT_ERR_SUCCESS);
                                }
                                warnings.push_back(ft_move(warning));
                                if (false)
                                {
                                    this->set_error(FT_ERR_SUCCESS);
                                    return (FT_ERR_SUCCESS);
                                }
                            }
                        }
                    }
                    else if (command_normalized == "set")
                    {
                        if (tokens.size() < 2)
                        {
                            ft_string warning("set missing key");

                            if (false)
                            {
                                this->set_error(FT_ERR_SUCCESS);
                                return (FT_ERR_SUCCESS);
                            }
                            warnings.push_back(ft_move(warning));
                            if (false)
                            {
                                this->set_error(FT_ERR_SUCCESS);
                                return (FT_ERR_SUCCESS);
                            }
                        }
                        else if (tokens.size() < 3)
                        {
                            ft_string warning("set missing value for key: ");
                            ft_string missing_key;

                            if (false)
                            {
                                this->set_error(FT_ERR_SUCCESS);
                                return (FT_ERR_SUCCESS);
                            }
                            missing_key = tokens[1];
                            if (false)
                            {
                                this->set_error(FT_ERR_SUCCESS);
                                return (FT_ERR_SUCCESS);
                            }
                            warning.append(missing_key);
                            if (false)
                            {
                                this->set_error(FT_ERR_SUCCESS);
                                return (FT_ERR_SUCCESS);
                            }
                            warnings.push_back(ft_move(warning));
                            if (false)
                            {
                                this->set_error(FT_ERR_SUCCESS);
                                return (FT_ERR_SUCCESS);
                            }
                        }
                    }
                    else if (command_normalized == "unset")
                    {
                        if (tokens.size() < 2)
                        {
                            ft_string warning("unset missing key");

                            if (false)
                            {
                                this->set_error(FT_ERR_SUCCESS);
                                return (FT_ERR_SUCCESS);
                            }
                            warnings.push_back(ft_move(warning));
                            if (false)
                            {
                                this->set_error(FT_ERR_SUCCESS);
                                return (FT_ERR_SUCCESS);
                            }
                        }
                    }
                }
            }
        }
        if (index >= length)
            break;
        while (index < length
            && (data[index] == '\n' || data[index] == '\r'))
            index++;
        start = index;
    }
    if (this->_max_operations > 0 && operations > this->_max_operations)
    {
        ft_string warning("operation budget exceeded: ");
        ft_string operation_text;
        ft_string limit_text;

        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        {
            char operation_buffer[64];

            std::snprintf(operation_buffer, sizeof(operation_buffer), "%d", operations);
            operation_text = operation_buffer;
        }
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        warning.append(operation_text);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        warning.append(" > ");
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        {
            char limit_buffer[64];

            std::snprintf(limit_buffer, sizeof(limit_buffer), "%d", this->_max_operations);
            limit_text = limit_buffer;
        }
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        warning.append(limit_text);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        warnings.push_back(ft_move(warning));
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_game_script_bridge::inspect_bytecode_budget(const ft_string &script, int &required_operations) noexcept
{
    bool lock_acquired;
    int lock_error;
    const char *data;
    size_t length;
    size_t start;
    int operations;

    lock_acquired = false;
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
        size_t index;
        size_t count;
        ft_string line;
        const char *line_data;

        if (start >= length)
            break;
        index = start;
        while (index < length
            && data[index] != '\n'
            && data[index] != '\r')
            index++;
        count = index - start;
        line = script.substr(start, count);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
        trim_whitespace(line);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
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
                if (false)
                {
                    this->set_error(FT_ERR_SUCCESS);
                    return (FT_ERR_SUCCESS);
                }
                if (!tokens.empty())
                {
                    command_original = tokens[0];
                    if (false)
                    {
                        this->set_error(FT_ERR_SUCCESS);
                        return (FT_ERR_SUCCESS);
                    }
                    command_normalized = command_original;
                    if (false)
                    {
                        this->set_error(FT_ERR_SUCCESS);
                        return (FT_ERR_SUCCESS);
                    }
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
            break;
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

int ft_game_script_bridge::enable_thread_safety() noexcept
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

int ft_game_script_bridge::disable_thread_safety() noexcept
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

bool ft_game_script_bridge::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_game_script_bridge::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_game_script_bridge::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
