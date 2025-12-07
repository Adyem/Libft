#include "ft_dialogue_line.hpp"
#include "game_narrative_helpers.hpp"
#include "../Template/move.hpp"

static void game_dialogue_copy_int_vector(const ft_vector<int> &source, ft_vector<int> &destination)
{
    int entry_errno;
    ft_vector<int>::const_iterator entry;
    ft_vector<int>::const_iterator end;

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

int ft_dialogue_line::lock_pair(const ft_dialogue_line &first, const ft_dialogue_line &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_dialogue_line *ordered_first;
    const ft_dialogue_line *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_dialogue_line *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
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
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
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

ft_dialogue_line::ft_dialogue_line() noexcept
    : _line_id(0), _speaker(), _text(), _next_line_ids(), _error_code(ER_SUCCESS)
{
    return ;
}

ft_dialogue_line::ft_dialogue_line(int line_id, const ft_string &speaker, const ft_string &text,
        const ft_vector<int> &next_line_ids) noexcept
    : _line_id(line_id), _speaker(speaker), _text(text), _next_line_ids(), _error_code(ER_SUCCESS)
{
    game_dialogue_copy_int_vector(next_line_ids, this->_next_line_ids);
    return ;
}

ft_dialogue_line::~ft_dialogue_line() noexcept
{
    return ;
}

ft_dialogue_line::ft_dialogue_line(const ft_dialogue_line &other) noexcept
    : _line_id(0), _speaker(), _text(), _next_line_ids(), _error_code(ER_SUCCESS)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_line_id = other._line_id;
    this->_speaker = other._speaker;
    this->_text = other._text;
    game_dialogue_copy_int_vector(other._next_line_ids, this->_next_line_ids);
    this->_error_code = other._error_code;
    game_narrative_restore_errno(other_guard, entry_errno);
    return ;
}

ft_dialogue_line &ft_dialogue_line::operator=(const ft_dialogue_line &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_dialogue_line::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_line_id = other._line_id;
    this->_speaker = other._speaker;
    this->_text = other._text;
    game_dialogue_copy_int_vector(other._next_line_ids, this->_next_line_ids);
    this->_error_code = other._error_code;
    game_narrative_restore_errno(this_guard, entry_errno);
    game_narrative_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_dialogue_line::ft_dialogue_line(ft_dialogue_line &&other) noexcept
    : _line_id(0), _speaker(), _text(), _next_line_ids(), _error_code(ER_SUCCESS)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_line_id = other._line_id;
    this->_speaker = other._speaker;
    this->_text = other._text;
    game_dialogue_copy_int_vector(other._next_line_ids, this->_next_line_ids);
    this->_error_code = other._error_code;
    other._line_id = 0;
    other._speaker.clear();
    other._text.clear();
    other._next_line_ids.clear();
    other._error_code = ER_SUCCESS;
    game_narrative_restore_errno(other_guard, entry_errno);
    return ;
}

ft_dialogue_line &ft_dialogue_line::operator=(ft_dialogue_line &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_dialogue_line::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_line_id = other._line_id;
    this->_speaker = other._speaker;
    this->_text = other._text;
    game_dialogue_copy_int_vector(other._next_line_ids, this->_next_line_ids);
    this->_error_code = other._error_code;
    other._line_id = 0;
    other._speaker.clear();
    other._text.clear();
    other._next_line_ids.clear();
    other._error_code = ER_SUCCESS;
    game_narrative_restore_errno(this_guard, entry_errno);
    game_narrative_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_dialogue_line::get_line_id() const noexcept
{
    int entry_errno;
    int line_id;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (0);
    }
    line_id = this->_line_id;
    game_narrative_restore_errno(guard, entry_errno);
    return (line_id);
}

void ft_dialogue_line::set_line_id(int line_id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_line_id = line_id;
    this->_error_code = ER_SUCCESS;
    game_narrative_restore_errno(guard, entry_errno);
    return ;
}

const ft_string &ft_dialogue_line::get_speaker() const noexcept
{
    int entry_errno;
    const ft_string *speaker;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (this->_speaker);
    }
    speaker = &this->_speaker;
    game_narrative_restore_errno(guard, entry_errno);
    return (*speaker);
}

void ft_dialogue_line::set_speaker(const ft_string &speaker) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_speaker = speaker;
    this->_error_code = ER_SUCCESS;
    game_narrative_restore_errno(guard, entry_errno);
    return ;
}

const ft_string &ft_dialogue_line::get_text() const noexcept
{
    int entry_errno;
    const ft_string *text;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (this->_text);
    }
    text = &this->_text;
    game_narrative_restore_errno(guard, entry_errno);
    return (*text);
}

void ft_dialogue_line::set_text(const ft_string &text) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_text = text;
    this->_error_code = ER_SUCCESS;
    game_narrative_restore_errno(guard, entry_errno);
    return ;
}

const ft_vector<int> &ft_dialogue_line::get_next_line_ids() const noexcept
{
    int entry_errno;
    const ft_vector<int> *next_lines;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (this->_next_line_ids);
    }
    next_lines = &this->_next_line_ids;
    game_narrative_restore_errno(guard, entry_errno);
    return (*next_lines);
}

ft_vector<int> &ft_dialogue_line::get_next_line_ids() noexcept
{
    int entry_errno;
    ft_vector<int> *next_lines;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (this->_next_line_ids);
    }
    next_lines = &this->_next_line_ids;
    game_narrative_restore_errno(guard, entry_errno);
    return (*next_lines);
}

void ft_dialogue_line::set_next_line_ids(const ft_vector<int> &next_line_ids) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    game_dialogue_copy_int_vector(next_line_ids, this->_next_line_ids);
    this->_error_code = ER_SUCCESS;
    game_narrative_restore_errno(guard, entry_errno);
    return ;
}

int ft_dialogue_line::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        return (guard.get_error());
    }
    error_code = this->_error_code;
    game_narrative_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_dialogue_line::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}

void ft_dialogue_line::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}
