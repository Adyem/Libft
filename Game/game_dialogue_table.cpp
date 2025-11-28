#include "game_dialogue_table.hpp"
#include "game_narrative_helpers.hpp"

int ft_dialogue_table::clone_from(const ft_dialogue_table &other) noexcept
{
    this->_lines = other._lines;
    if (this->_lines.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_lines.get_error());
        return (this->_lines.get_error());
    }
    this->_scripts = other._scripts;
    if (this->_scripts.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_scripts.get_error());
        return (this->_scripts.get_error());
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

ft_dialogue_table::ft_dialogue_table() noexcept
    : _lines(), _scripts(), _error_code(ER_SUCCESS)
{
    return ;
}

ft_dialogue_table::~ft_dialogue_table() noexcept
{
    return ;
}

ft_dialogue_table::ft_dialogue_table(const ft_dialogue_table &other) noexcept
    : _lines(), _scripts(), _error_code(ER_SUCCESS)
{
    int entry_errno;
    int other_error_code;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    entry_errno = ft_errno;
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(other_guard, entry_errno);
        return ;
    }
    other_error_code = other._error_code;
    if (this->clone_from(other) != ER_SUCCESS)
    {
        game_narrative_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_error_code = other_error_code;
    this->set_error(this->_error_code);
    game_narrative_restore_errno(other_guard, entry_errno);
    return ;
}

ft_dialogue_table &ft_dialogue_table::operator=(const ft_dialogue_table &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int other_error_code;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    this_guard = ft_unique_lock<pt_mutex>(this->_mutex);
    if (this_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(this_guard.get_error());
        return (*this);
    }
    other_guard = ft_unique_lock<pt_mutex>(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(this_guard, entry_errno);
        return (*this);
    }
    other_error_code = other._error_code;
    if (this->clone_from(other) != ER_SUCCESS)
    {
        game_narrative_restore_errno(this_guard, entry_errno);
        game_narrative_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_error_code = other_error_code;
    this->set_error(this->_error_code);
    game_narrative_restore_errno(this_guard, entry_errno);
    game_narrative_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_dialogue_table::ft_dialogue_table(ft_dialogue_table &&other) noexcept
    : _lines(), _scripts(), _error_code(ER_SUCCESS)
{
    int entry_errno;
    int other_error_code;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    entry_errno = ft_errno;
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(other_guard, entry_errno);
        return ;
    }
    other_error_code = other._error_code;
    if (this->clone_from(other) != ER_SUCCESS)
    {
        game_narrative_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_error_code = other_error_code;
    other._lines.clear();
    other._scripts.clear();
    other._error_code = ER_SUCCESS;
    other.set_error(ER_SUCCESS);
    this->set_error(this->_error_code);
    game_narrative_restore_errno(other_guard, entry_errno);
    return ;
}

ft_dialogue_table &ft_dialogue_table::operator=(ft_dialogue_table &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int other_error_code;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    this_guard = ft_unique_lock<pt_mutex>(this->_mutex);
    if (this_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(this_guard.get_error());
        return (*this);
    }
    other_guard = ft_unique_lock<pt_mutex>(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(this_guard, entry_errno);
        return (*this);
    }
    other_error_code = other._error_code;
    if (this->clone_from(other) != ER_SUCCESS)
    {
        game_narrative_restore_errno(this_guard, entry_errno);
        game_narrative_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_error_code = other_error_code;
    other._lines.clear();
    other._scripts.clear();
    other._error_code = ER_SUCCESS;
    other.set_error(ER_SUCCESS);
    this->set_error(this->_error_code);
    game_narrative_restore_errno(this_guard, entry_errno);
    game_narrative_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_dialogue_table::register_line(const ft_dialogue_line &line) noexcept
{
    int entry_errno;
    int identifier;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    if (line.get_error() != ER_SUCCESS)
    {
        this->set_error(line.get_error());
        game_narrative_restore_errno(guard, entry_errno);
        return (line.get_error());
    }
    identifier = line.get_line_id();
    this->_lines.insert(identifier, line);
    this->set_error(this->_lines.get_error());
    game_narrative_restore_errno(guard, entry_errno);
    return (this->_error_code);
}

int ft_dialogue_table::register_script(const ft_dialogue_script &script) noexcept
{
    int entry_errno;
    int identifier;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (guard.get_error());
    }
    if (script.get_error() != ER_SUCCESS)
    {
        this->set_error(script.get_error());
        game_narrative_restore_errno(guard, entry_errno);
        return (script.get_error());
    }
    identifier = script.get_script_id();
    this->_scripts.insert(identifier, script);
    this->set_error(this->_scripts.get_error());
    game_narrative_restore_errno(guard, entry_errno);
    return (this->_error_code);
}

int ft_dialogue_table::fetch_line(int line_id, ft_dialogue_line &out_line) const noexcept
{
    int entry_errno;
    const Pair<int, ft_dialogue_line> *entry;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_dialogue_table *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    entry = this->_lines.find(line_id);
    if (entry == this->_lines.end())
    {
        const_cast<ft_dialogue_table *>(this)->set_error(FT_ERR_NOT_FOUND);
        game_narrative_restore_errno(guard, entry_errno);
        return (FT_ERR_NOT_FOUND);
    }
    out_line = entry->value;
    const_cast<ft_dialogue_table *>(this)->set_error(out_line.get_error());
    game_narrative_restore_errno(guard, entry_errno);
    return (out_line.get_error());
}

int ft_dialogue_table::fetch_script(int script_id, ft_dialogue_script &out_script) const noexcept
{
    int entry_errno;
    const Pair<int, ft_dialogue_script> *entry;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_dialogue_table *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    entry = this->_scripts.find(script_id);
    if (entry == this->_scripts.end())
    {
        const_cast<ft_dialogue_table *>(this)->set_error(FT_ERR_NOT_FOUND);
        game_narrative_restore_errno(guard, entry_errno);
        return (FT_ERR_NOT_FOUND);
    }
    out_script = entry->value;
    const_cast<ft_dialogue_table *>(this)->set_error(out_script.get_error());
    game_narrative_restore_errno(guard, entry_errno);
    return (out_script.get_error());
}

ft_map<int, ft_dialogue_line> &ft_dialogue_table::get_lines() noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_lines);
}

const ft_map<int, ft_dialogue_line> &ft_dialogue_table::get_lines() const noexcept
{
    const_cast<ft_dialogue_table *>(this)->set_error(ER_SUCCESS);
    return (this->_lines);
}

void ft_dialogue_table::set_lines(const ft_map<int, ft_dialogue_line> &lines) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_lines = lines;
    this->_error_code = this->_lines.get_error();
    game_narrative_restore_errno(guard, entry_errno);
    return ;
}

ft_map<int, ft_dialogue_script> &ft_dialogue_table::get_scripts() noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_scripts);
}

const ft_map<int, ft_dialogue_script> &ft_dialogue_table::get_scripts() const noexcept
{
    const_cast<ft_dialogue_table *>(this)->set_error(ER_SUCCESS);
    return (this->_scripts);
}

void ft_dialogue_table::set_scripts(const ft_map<int, ft_dialogue_script> &scripts) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_scripts = scripts;
    this->_error_code = this->_scripts.get_error();
    game_narrative_restore_errno(guard, entry_errno);
    return ;
}

int ft_dialogue_table::get_error() const noexcept
{
    int entry_errno;
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        return (guard.get_error());
    }
    error_code = this->_error_code;
    game_narrative_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_dialogue_table::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}

void ft_dialogue_table::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}
