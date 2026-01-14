#include "ft_dialogue_script.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"

static void game_dialogue_copy_line_vector(const ft_vector<ft_dialogue_line> &source,
        ft_vector<ft_dialogue_line> &destination)
{
    ft_vector<ft_dialogue_line>::const_iterator entry;
    ft_vector<ft_dialogue_line>::const_iterator end;

    destination.clear();
    entry = source.begin();
    end = source.end();
    while (entry != end)
    {
        destination.push_back(*entry);
        ++entry;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(single_guard.get_error());
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
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

        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(lower_guard.get_error());
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
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_global_error_stack_push(upper_guard.get_error());
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        pt_thread_sleep(1);
    }
}

ft_dialogue_script::ft_dialogue_script() noexcept
    : _script_id(0), _title(), _summary(), _start_line_id(0), _lines(),
    _error_code(FT_ERR_SUCCESSS)
{
    return ;
}

ft_dialogue_script::ft_dialogue_script(int script_id, const ft_string &title, const ft_string &summary,
        int start_line_id, const ft_vector<ft_dialogue_line> &lines) noexcept
    : _script_id(script_id), _title(title), _summary(summary),
    _start_line_id(start_line_id),
    _lines(), _error_code(FT_ERR_SUCCESSS)
{
    game_dialogue_copy_line_vector(lines, this->_lines);
    return ;
}

ft_dialogue_script::~ft_dialogue_script() noexcept
{
    return ;
}

ft_dialogue_script::ft_dialogue_script(const ft_dialogue_script &other) noexcept
    : _script_id(0), _title(), _summary(), _start_line_id(0),
    _lines(), _error_code(FT_ERR_SUCCESSS)
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_script_id = other._script_id;
    this->_title = other._title;
    this->_summary = other._summary;
    this->_start_line_id = other._start_line_id;
    game_dialogue_copy_line_vector(other._lines, this->_lines);
    this->_error_code = other._error_code;
    return ;
}

ft_dialogue_script &ft_dialogue_script::operator=(const ft_dialogue_script &other)
    noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_dialogue_script::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
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
    return (*this);
}

ft_dialogue_script::ft_dialogue_script(ft_dialogue_script &&other) noexcept
    : _script_id(0), _title(), _summary(), _start_line_id(0), _lines(),
    _error_code(FT_ERR_SUCCESSS)
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
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
    other._error_code = FT_ERR_SUCCESSS;
    return ;
}

ft_dialogue_script &ft_dialogue_script::operator=(ft_dialogue_script &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_dialogue_script::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
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
    other._error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

int ft_dialogue_script::get_script_id() const noexcept
{
    int script_id;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (0);
    }
    script_id = this->_script_id;
    return (script_id);
}

void ft_dialogue_script::set_script_id(int script_id) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_script_id = script_id;
    this->_error_code = FT_ERR_SUCCESSS;
    return ;
}

const ft_string &ft_dialogue_script::get_title() const noexcept
{
    const ft_string *title;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (this->_title);
    }
    title = &this->_title;
    return (*title);
}

void ft_dialogue_script::set_title(const ft_string &title) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_title = title;
    this->_error_code = FT_ERR_SUCCESSS;
    return ;
}

const ft_string &ft_dialogue_script::get_summary() const noexcept
{
    const ft_string *summary;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (this->_summary);
    }
    summary = &this->_summary;
    return (*summary);
}

void ft_dialogue_script::set_summary(const ft_string &summary) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_summary = summary;
    this->_error_code = FT_ERR_SUCCESSS;
    return ;
}

int ft_dialogue_script::get_start_line_id() const noexcept
{
    int start_line_id;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (0);
    }
    start_line_id = this->_start_line_id;
    return (start_line_id);
}

void ft_dialogue_script::set_start_line_id(int start_line_id) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_start_line_id = start_line_id;
    this->_error_code = FT_ERR_SUCCESSS;
    return ;
}

const ft_vector<ft_dialogue_line> &ft_dialogue_script::get_lines() const noexcept
{
    const ft_vector<ft_dialogue_line> *lines;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (this->_lines);
    }
    lines = &this->_lines;
    return (*lines);
}

ft_vector<ft_dialogue_line> &ft_dialogue_script::get_lines() noexcept
{
    ft_vector<ft_dialogue_line> *lines;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (this->_lines);
    }
    lines = &this->_lines;
    return (*lines);
}

void ft_dialogue_script::set_lines(const ft_vector<ft_dialogue_line> &lines) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    game_dialogue_copy_line_vector(lines, this->_lines);
    this->_error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int ft_dialogue_script::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(guard.get_error());
        return (guard.get_error());
    }
    error_code = this->_error_code;
    ft_global_error_stack_push(error_code);
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
    ft_global_error_stack_push(error_code);
    return ;
}
