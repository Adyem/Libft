#include "ft_dialogue_line.hpp"

static void game_dialogue_copy_int_vector(const ft_vector<int> &source,
    ft_vector<int> &destination)
{
    ft_vector<int>::const_iterator entry;
    ft_vector<int>::const_iterator end;

    destination.clear();
    entry = source.begin();
    end = source.end();
    while (entry != end)
    {
        destination.push_back(*entry);
        ++entry;
    }
    return ;
}

ft_dialogue_line::ft_dialogue_line() noexcept
    : _line_id(0), _speaker(), _text(), _next_line_ids(), _mutex()
{
    return ;
}

ft_dialogue_line::ft_dialogue_line(int line_id, const ft_string &speaker,
    const ft_string &text, const ft_vector<int> &next_line_ids) noexcept
    : _line_id(line_id), _speaker(speaker), _text(text), _next_line_ids(), _mutex()
{
    game_dialogue_copy_int_vector(next_line_ids, this->_next_line_ids);
    return ;
}

ft_dialogue_line::~ft_dialogue_line() noexcept
{
    return ;
}

int ft_dialogue_line::get_line_id() const noexcept
{
    int line_id;

    if (this->_mutex.lock() != FT_ERR_SUCCESS)
        return (0);
    line_id = this->_line_id;
    this->_mutex.unlock();
    return (line_id);
}

void ft_dialogue_line::set_line_id(int line_id) noexcept
{
    if (this->_mutex.lock() != FT_ERR_SUCCESS)
        return ;
    this->_line_id = line_id;
    this->_mutex.unlock();
    return ;
}

const ft_string &ft_dialogue_line::get_speaker() const noexcept
{
    const ft_string *speaker_pointer = &this->_speaker;

    if (this->_mutex.lock() != FT_ERR_SUCCESS)
        return (*speaker_pointer);
    speaker_pointer = &this->_speaker;
    this->_mutex.unlock();
    return (*speaker_pointer);
}

void ft_dialogue_line::set_speaker(const ft_string &speaker) noexcept
{
    if (this->_mutex.lock() != FT_ERR_SUCCESS)
        return ;
    this->_speaker = speaker;
    this->_mutex.unlock();
    return ;
}

const ft_string &ft_dialogue_line::get_text() const noexcept
{
    const ft_string *text_pointer = &this->_text;

    if (this->_mutex.lock() != FT_ERR_SUCCESS)
        return (*text_pointer);
    text_pointer = &this->_text;
    this->_mutex.unlock();
    return (*text_pointer);
}

void ft_dialogue_line::set_text(const ft_string &text) noexcept
{
    if (this->_mutex.lock() != FT_ERR_SUCCESS)
        return ;
    this->_text = text;
    this->_mutex.unlock();
    return ;
}

const ft_vector<int> &ft_dialogue_line::get_next_line_ids() const noexcept
{
    const ft_vector<int> *next_lines = &this->_next_line_ids;

    if (this->_mutex.lock() != FT_ERR_SUCCESS)
        return (*next_lines);
    next_lines = &this->_next_line_ids;
    this->_mutex.unlock();
    return (*next_lines);
}

ft_vector<int> &ft_dialogue_line::get_next_line_ids() noexcept
{
    if (this->_mutex.lock() != FT_ERR_SUCCESS)
        return (this->_next_line_ids);
    ft_vector<int> &next_lines = this->_next_line_ids;
    this->_mutex.unlock();
    return (next_lines);
}

void ft_dialogue_line::set_next_line_ids(const ft_vector<int> &next_line_ids) noexcept
{
    if (this->_mutex.lock() != FT_ERR_SUCCESS)
        return ;
    game_dialogue_copy_int_vector(next_line_ids, this->_next_line_ids);
    this->_mutex.unlock();
    return ;
}

int ft_dialogue_line::get_error() const noexcept
{
    return (FT_ERR_SUCCESS);
}

const char *ft_dialogue_line::get_error_str() const noexcept
{
    return (ft_strerror(FT_ERR_SUCCESS));
}
