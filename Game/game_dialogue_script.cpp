#include "ft_dialogue_script.hpp"
#include "game_narrative_helpers.hpp"
#include "../Template/move.hpp"

static void game_dialogue_copy_line_vector(const ft_vector<ft_dialogue_line> &source,
        ft_vector<ft_dialogue_line> &destination)
{
    int entry_errno;
    ft_vector<ft_dialogue_line>::const_iterator entry;
    ft_vector<ft_dialogue_line>::const_iterator end;

    entry_errno = ft_errno;
    destination.clear();
    entry = source.begin();
    end = source.end();
    while (entry != end)
    {
        destination.push_back(*entry);
        ++entry;
    }
    ft_errno = entry_errno;
    return ;
}

int ft_dialogue_script::lock_pair(const ft_dialogue_script &first, const ft_dialogue_script &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_dialogue_script *ordered_first;
    const ft_dialogue_script *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ER_SUCCESSS;
        return (FT_ER_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_dialogue_script *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ER_SUCCESSS)
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
            ft_errno = FT_ER_SUCCESSS;
            return (FT_ER_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_narrative_sleep_backoff();
    }
}

ft_dialogue_script::ft_dialogue_script() noexcept
    : _script_id(0), _title(), _summary(), _start_line_id(0), _lines(), _error_code(FT_ER_SUCCESSS)
{
    return ;
}

ft_dialogue_script::ft_dialogue_script(int script_id, const ft_string &title, const ft_string &summary,
        int start_line_id, const ft_vector<ft_dialogue_line> &lines) noexcept
    : _script_id(script_id), _title(title), _summary(summary), _start_line_id(start_line_id),
    _lines(), _error_code(FT_ER_SUCCESSS)
{
    game_dialogue_copy_line_vector(lines, this->_lines);
    return ;
}

ft_dialogue_script::~ft_dialogue_script() noexcept
{
    return ;
}

ft_dialogue_script::ft_dialogue_script(const ft_dialogue_script &other) noexcept
    : _script_id(0), _title(), _summary(), _start_line_id(0), _lines(), _error_code(FT_ER_SUCCESSS)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_script_id = other._script_id;
    this->_title = other._title;
    this->_summary = other._summary;
    this->_start_line_id = other._start_line_id;
    game_dialogue_copy_line_vector(other._lines, this->_lines);
    this->_error_code = other._error_code;
    game_narrative_restore_errno(other_guard, entry_errno);
    return ;
}

ft_dialogue_script &ft_dialogue_script::operator=(const ft_dialogue_script &other) noexcept
{
    if (this == &other)
        return (*this);
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;
    lock_error = ft_dialogue_script::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_script_id = other._script_id;
    this->_title = other._title;
    this->_summary = other._summary;
    this->_start_line_id = other._start_line_id;
    game_dialogue_copy_line_vector(other._lines, this->_lines);
    this->_error_code = other._error_code;
    game_narrative_restore_errno(this_guard, FT_ER_SUCCESSS);
    game_narrative_restore_errno(other_guard, FT_ER_SUCCESSS);
    return (*this);
}

ft_dialogue_script::ft_dialogue_script(ft_dialogue_script &&other) noexcept
    : _script_id(0), _title(), _summary(), _start_line_id(0), _lines(), _error_code(FT_ER_SUCCESSS)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_script_id = other._script_id;
    this->_title = other._title;
    this->_summary = other._summary;
    this->_start_line_id = other._start_line_id;
    game_dialogue_copy_line_vector(other._lines, this->_lines);
    this->_error_code = other._error_code;
    other._script_id = 0;
    other._title.clear();
    other._summary.clear();
    other._start_line_id = 0;
    other._lines.clear();
    other._error_code = FT_ER_SUCCESSS;
    game_narrative_restore_errno(other_guard, entry_errno);
    return ;
}

ft_dialogue_script &ft_dialogue_script::operator=(ft_dialogue_script &&other) noexcept
{
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    lock_error = ft_dialogue_script::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_script_id = other._script_id;
    this->_title = other._title;
    this->_summary = other._summary;
    this->_start_line_id = other._start_line_id;
    game_dialogue_copy_line_vector(other._lines, this->_lines);
    this->_error_code = other._error_code;
    other._script_id = 0;
    other._title.clear();
    other._summary.clear();
    other._start_line_id = 0;
    other._lines.clear();
    other._error_code = FT_ER_SUCCESSS;
    game_narrative_restore_errno(this_guard, entry_errno);
    game_narrative_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_dialogue_script::get_script_id() const noexcept
{
    int entry_errno;
    int script_id;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (0);
    }
    script_id = this->_script_id;
    game_narrative_restore_errno(guard, FT_ER_SUCCESSS);
    return (script_id);
}

void ft_dialogue_script::set_script_id(int script_id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_script_id = script_id;
    this->_error_code = FT_ER_SUCCESSS;
    game_narrative_restore_errno(guard, FT_ER_SUCCESSS);
    return ;
}

const ft_string &ft_dialogue_script::get_title() const noexcept
{
    int entry_errno;
    const ft_string *title;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (this->_title);
    }
    title = &this->_title;
    game_narrative_restore_errno(guard, FT_ER_SUCCESSS);
    return (*title);
}

void ft_dialogue_script::set_title(const ft_string &title) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_title = title;
    this->_error_code = FT_ER_SUCCESSS;
    game_narrative_restore_errno(guard, FT_ER_SUCCESSS);
    return ;
}

const ft_string &ft_dialogue_script::get_summary() const noexcept
{
    int entry_errno;
    const ft_string *summary;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (this->_summary);
    }
    summary = &this->_summary;
    game_narrative_restore_errno(guard, FT_ER_SUCCESSS);
    return (*summary);
}

void ft_dialogue_script::set_summary(const ft_string &summary) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_summary = summary;
    this->_error_code = FT_ER_SUCCESSS;
    game_narrative_restore_errno(guard, FT_ER_SUCCESSS);
    return ;
}

int ft_dialogue_script::get_start_line_id() const noexcept
{
    int entry_errno;
    int start_line_id;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (0);
    }
    start_line_id = this->_start_line_id;
    game_narrative_restore_errno(guard, FT_ER_SUCCESSS);
    return (start_line_id);
}

void ft_dialogue_script::set_start_line_id(int start_line_id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_start_line_id = start_line_id;
    this->_error_code = FT_ER_SUCCESSS;
    game_narrative_restore_errno(guard, FT_ER_SUCCESSS);
    return ;
}

const ft_vector<ft_dialogue_line> &ft_dialogue_script::get_lines() const noexcept
{
    int entry_errno;
    const ft_vector<ft_dialogue_line> *lines;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (this->_lines);
    }
    lines = &this->_lines;
    game_narrative_restore_errno(guard, FT_ER_SUCCESSS);
    return (*lines);
}

ft_vector<ft_dialogue_line> &ft_dialogue_script::get_lines() noexcept
{
    int entry_errno;
    ft_vector<ft_dialogue_line> *lines;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (this->_lines);
    }
    lines = &this->_lines;
    game_narrative_restore_errno(guard, FT_ER_SUCCESSS);
    return (*lines);
}

void ft_dialogue_script::set_lines(const ft_vector<ft_dialogue_line> &lines) noexcept
{
    int entry_errno;
    bool had_lines;

    entry_errno = ft_errno;
    had_lines = (this->_lines.empty() == false);
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    game_dialogue_copy_line_vector(lines, this->_lines);
    this->_error_code = FT_ER_SUCCESSS;
    if (had_lines == true)
        game_narrative_restore_errno(guard, FT_ER_SUCCESSS);
    else
        game_narrative_restore_errno(guard, entry_errno);
    return ;
}

int ft_dialogue_script::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = entry_errno;
        return (guard.get_error());
    }
    error_code = this->_error_code;
    game_narrative_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_dialogue_script::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}

void ft_dialogue_script::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}
