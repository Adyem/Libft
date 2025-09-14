#include "event_scheduler.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include <cstdio>

bool ft_event_compare::operator()(const ft_event &left, const ft_event &right) const noexcept
{
    return (left.get_duration() > right.get_duration());
}

ft_event_scheduler::ft_event_scheduler() noexcept
    : _events(), _error_code(ER_SUCCESS)
{
    if (this->_events.get_error() != ER_SUCCESS)
        this->set_error(this->_events.get_error());
    return ;
}

ft_event_scheduler::~ft_event_scheduler()
{
    return ;
}

void ft_event_scheduler::set_error(int error) const noexcept
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

void ft_event_scheduler::schedule_event(const ft_event &event) noexcept
{
    this->_events.push(event);
    if (this->_events.get_error() != ER_SUCCESS)
        this->set_error(this->_events.get_error());
    return ;
}

void ft_event_scheduler::update_events(int ticks, const char *log_file_path, ft_string *log_buffer) noexcept
{
    ft_priority_queue<ft_event, ft_event_compare> temp;
    ft_event current_event;
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            return ;
        }
        current_event.sub_duration(ticks);
        if (current_event.get_duration() <= 0)
        {
            if (log_file_path)
                log_event_to_file(current_event, log_file_path);
            if (log_buffer)
                log_event_to_buffer(current_event, *log_buffer);
        }
        else
            temp.push(current_event);
    }
    while (!temp.empty())
    {
        this->_events.push(temp.pop());
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            return ;
        }
    }
    return ;
}

void ft_event_scheduler::dump_events(ft_vector<ft_event> &out) const noexcept
{
    ft_priority_queue<ft_event, ft_event_compare> temp;
    ft_event current_event;
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        out.push_back(current_event);
        temp.push(current_event);
    }
    while (!temp.empty())
        this->_events.push(temp.pop());
    return ;
}

size_t ft_event_scheduler::size() const noexcept
{
    return (this->_events.size());
}

void ft_event_scheduler::clear() noexcept
{
    this->_events.clear();
    if (this->_events.get_error() != ER_SUCCESS)
        this->set_error(this->_events.get_error());
    return ;
}

int ft_event_scheduler::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_event_scheduler::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

int log_event_to_file(const ft_event &event, const char *file_path) noexcept
{
    FILE *file = fopen(file_path, "a");
    if (!file)
        return (-1);
    fprintf(file, "event %d processed\n", event.get_id());
    fclose(file);
    return (ER_SUCCESS);
}

void log_event_to_buffer(const ft_event &event, ft_string &buffer) noexcept
{
    char *id_string = cma_itoa(event.get_id());
    if (id_string)
    {
        buffer += "event ";
        buffer += id_string;
        buffer += " processed\n";
        cma_free(id_string);
    }
    return ;
}
