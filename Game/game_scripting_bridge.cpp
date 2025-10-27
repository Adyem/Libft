#include "game_scripting_bridge.hpp"
#include "../Template/pair.hpp"
#include "../Template/move.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/unique_lock.hpp"

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
    ft_errno = error;
    this->_error_code = error;
    return ;
}

ft_game_script_context::ft_game_script_context() noexcept
    : _state(ft_nullptr), _world(), _variables(), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_game_script_context::ft_game_script_context(ft_game_state *state, const ft_sharedptr<ft_world> &world) noexcept
    : _state(state), _world(world), _variables(), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_game_script_context::~ft_game_script_context() noexcept
{
    return ;
}

ft_game_script_context::ft_game_script_context(const ft_game_script_context &other) noexcept
    : _state(other._state), _world(other._world), _variables(other._variables), _error_code(other._error_code)
{
    this->set_error(other._error_code);
    return ;
}

ft_game_script_context &ft_game_script_context::operator=(const ft_game_script_context &other) noexcept
{
    if (this != &other)
    {
        this->_state = other._state;
        this->_world = other._world;
        this->_variables = other._variables;
        this->set_error(other._error_code);
    }
    return (*this);
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
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_game_script_context::set_world(const ft_sharedptr<ft_world> &world) noexcept
{
    this->_world = world;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_game_script_context::set_variable(const ft_string &key, const ft_string &value) noexcept
{
    Pair<ft_string, ft_string> *entry;

    entry = this->_variables.find(key);
    if (this->_variables.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_variables.get_error());
        return ;
    }
    if (entry != ft_nullptr)
    {
        entry->value = value;
        this->set_error(ER_SUCCESS);
        return ;
    }
    this->_variables.insert(key, value);
    if (this->_variables.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_variables.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

const ft_string *ft_game_script_context::get_variable(const ft_string &key) const noexcept
{
    const Pair<ft_string, ft_string> *entry;

    entry = this->_variables.find(key);
    if (this->_variables.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_variables.get_error());
        return (ft_nullptr);
    }
    if (entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (&entry->value);
}

void ft_game_script_context::remove_variable(const ft_string &key) noexcept
{
    this->_variables.remove(key);
    if (this->_variables.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_variables.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_game_script_context::clear_variables() noexcept
{
    this->_variables.clear();
    if (this->_variables.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_variables.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
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
    ft_errno = error;
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

ft_game_script_bridge::ft_game_script_bridge(const ft_sharedptr<ft_world> &world, const char *language) noexcept
    : _world(world), _callbacks(), _language(), _max_operations(32), _error_code(ER_SUCCESS), _mutex()
{
    if (language)
        this->_language = language;
    else
        this->_language = "lua";
    if (this->_language.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_language.get_error());
        return ;
    }
    if (ft_game_script_bridge::is_supported_language(this->_language) == false)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

ft_game_script_bridge::~ft_game_script_bridge() noexcept
{
    return ;
}

ft_game_script_bridge::ft_game_script_bridge(const ft_game_script_bridge &other) noexcept
    : _world(other._world), _callbacks(other._callbacks), _language(other._language), _max_operations(other._max_operations), _error_code(other._error_code), _mutex()
{
    this->set_error(other._error_code);
    return ;
}

ft_game_script_bridge &ft_game_script_bridge::operator=(const ft_game_script_bridge &other) noexcept
{
    if (this != &other)
    {
        ft_unique_lock<pt_mutex> guard_this(this->_mutex);

        this->_world = other._world;
        this->_callbacks = other._callbacks;
        this->_language = other._language;
        this->_max_operations = other._max_operations;
        this->set_error(other._error_code);
    }
    return (*this);
}

ft_game_script_bridge::ft_game_script_bridge(ft_game_script_bridge &&other) noexcept
    : _world(ft_move(other._world)), _callbacks(ft_move(other._callbacks)), _language(ft_move(other._language)), _max_operations(other._max_operations), _error_code(other._error_code), _mutex()
{
    other._max_operations = 0;
    other.set_error(ER_SUCCESS);
    this->set_error(this->_error_code);
    return ;
}

ft_game_script_bridge &ft_game_script_bridge::operator=(ft_game_script_bridge &&other) noexcept
{
    if (this != &other)
    {
        ft_unique_lock<pt_mutex> guard_other(other._mutex);
        ft_unique_lock<pt_mutex> guard_this(this->_mutex);

        this->_world = ft_move(other._world);
        this->_callbacks = ft_move(other._callbacks);
        this->_language = ft_move(other._language);
        this->_max_operations = other._max_operations;
        this->set_error(other._error_code);
        other._max_operations = 0;
        other.set_error(ER_SUCCESS);
    }
    return (*this);
}

void ft_game_script_bridge::set_language(const char *language) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (!language)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ft_string candidate(language);
    if (candidate.get_error() != ER_SUCCESS)
    {
        this->set_error(candidate.get_error());
        return ;
    }
    if (ft_game_script_bridge::is_supported_language(candidate) == false)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_language = candidate;
    this->set_error(ER_SUCCESS);
    return ;
}

const ft_string &ft_game_script_bridge::get_language() const noexcept
{
    return (this->_language);
}

void ft_game_script_bridge::set_max_operations(int limit) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (limit < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_max_operations = limit;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_game_script_bridge::get_max_operations() const noexcept
{
    return (this->_max_operations);
}

int ft_game_script_bridge::register_function(const ft_string &name, const ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> &callback) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    Pair<ft_string, ft_function<int(ft_game_script_context &, const ft_vector<ft_string> &)> > *entry;

    if (name.empty() || !callback)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    entry = this->_callbacks.find(name);
    if (this->_callbacks.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_callbacks.get_error());
        return (this->_callbacks.get_error());
    }
    if (entry != ft_nullptr)
    {
        entry->value = callback;
        this->set_error(ER_SUCCESS);
        return (ER_SUCCESS);
    }
    this->_callbacks.insert(name, callback);
    if (this->_callbacks.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_callbacks.get_error());
        return (this->_callbacks.get_error());
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_game_script_bridge::remove_function(const ft_string &name) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    this->_callbacks.remove(name);
    if (this->_callbacks.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_callbacks.get_error());
        return (this->_callbacks.get_error());
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

void ft_game_script_bridge::tokenize_line(const ft_string &line, ft_vector<ft_string> &tokens) const noexcept
{
    const char *data;
    size_t length;
    size_t index;

    tokens.clear();
    if (tokens.get_error() != ER_SUCCESS)
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
        if (token.get_error() != ER_SUCCESS)
        {
            tokens.clear();
            return ;
        }
        tokens.push_back(ft_move(token));
        if (tokens.get_error() != ER_SUCCESS)
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
    if (value.get_error() != ER_SUCCESS)
    {
        this->set_error(value.get_error());
        return (value.get_error());
    }
    count = tokens.size();
    index = 3;
    while (index < count)
    {
        value.append(" ");
        if (value.get_error() != ER_SUCCESS)
        {
            this->set_error(value.get_error());
            return (value.get_error());
        }
        value.append(tokens[index]);
        if (value.get_error() != ER_SUCCESS)
        {
            this->set_error(value.get_error());
            return (value.get_error());
        }
        index++;
    }
    context.set_variable(tokens[1], value);
    if (context.get_error() != ER_SUCCESS)
    {
        this->set_error(context.get_error());
        return (context.get_error());
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_game_script_bridge::handle_unset(ft_game_script_context &context, const ft_vector<ft_string> &tokens) noexcept
{
    if (tokens.size() < 2)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    context.remove_variable(tokens[1]);
    if (context.get_error() != ER_SUCCESS)
    {
        this->set_error(context.get_error());
        return (context.get_error());
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
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
    entry = this->_callbacks.find(tokens[1]);
    if (this->_callbacks.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_callbacks.get_error());
        return (this->_callbacks.get_error());
    }
    if (entry == ft_nullptr)
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
        if (argument.get_error() != ER_SUCCESS)
        {
            this->set_error(argument.get_error());
            return (argument.get_error());
        }
        arguments.push_back(ft_move(argument));
        if (arguments.get_error() != ER_SUCCESS)
        {
            this->set_error(arguments.get_error());
            return (arguments.get_error());
        }
        index++;
    }
    result = entry->value(context, arguments);
    if (entry->value.get_error() != ER_SUCCESS)
    {
        this->set_error(entry->value.get_error());
        return (entry->value.get_error());
    }
    if (result != ER_SUCCESS)
    {
        this->set_error(result);
        return (result);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_game_script_bridge::execute_line(ft_game_script_context &context, const ft_string &line) noexcept
{
    ft_vector<ft_string> tokens;
    ft_string command;
    char *command_data;

    this->tokenize_line(line, tokens);
    if (tokens.get_error() != ER_SUCCESS)
    {
        this->set_error(tokens.get_error());
        return (tokens.get_error());
    }
    if (tokens.empty())
    {
        this->set_error(ER_SUCCESS);
        return (ER_SUCCESS);
    }
    command = tokens[0];
    if (command.get_error() != ER_SUCCESS)
    {
        this->set_error(command.get_error());
        return (command.get_error());
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
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    ft_game_script_context context(&state, this->_world);
    const char *data;
    size_t length;
    size_t start;
    int operations;

    if (context.get_error() != ER_SUCCESS)
    {
        this->set_error(context.get_error());
        return (context.get_error());
    }
    if (ft_game_script_bridge::is_supported_language(this->_language) == false)
    {
        this->set_error(FT_ERR_CONFIGURATION);
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
        if (line.get_error() != ER_SUCCESS)
        {
            this->set_error(line.get_error());
            return (line.get_error());
        }
        trim_whitespace(line);
        if (line.get_error() != ER_SUCCESS)
        {
            this->set_error(line.get_error());
            return (line.get_error());
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
                    return (FT_ERR_INVALID_OPERATION);
                }
                int result = this->execute_line(context, line);
                if (result != ER_SUCCESS)
                {
                    this->set_error(result);
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
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_game_script_bridge::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_game_script_bridge::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
