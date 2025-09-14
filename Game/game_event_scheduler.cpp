#include "game_event_scheduler.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../JSon/json.hpp"
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

static int add_item_field(json_group *group, const ft_string &key, int value)
{
    json_item *json_item_ptr = json_create_item(key.c_str(), value);
    if (!json_item_ptr)
    {
        json_free_groups(group);
        return (JSON_MALLOC_FAIL);
    }
    json_add_item_to_group(group, json_item_ptr);
    return (ER_SUCCESS);
}

json_group *serialize_event_scheduler(const ft_event_scheduler &scheduler)
{
    json_group *group = json_create_json_group("world");
    if (!group)
        return (ft_nullptr);
    json_item *count_item = json_create_item("event_count", static_cast<int>(scheduler.size()));
    if (!count_item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, count_item);
    ft_vector<ft_event> events;
    scheduler.dump_events(events);
    size_t event_index = 0;
    size_t event_count = events.size();
    while (event_index < event_count)
    {
        char *event_index_string = cma_itoa(static_cast<int>(event_index));
        if (!event_index_string)
        {
            json_free_groups(group);
            return (ft_nullptr);
        }
        ft_string key_id = "event_";
        key_id += event_index_string;
        key_id += "_id";
        ft_string key_duration = "event_";
        key_duration += event_index_string;
        key_duration += "_duration";
        cma_free(event_index_string);
        if (add_item_field(group, key_id, events[event_index].get_id()) != ER_SUCCESS ||
            add_item_field(group, key_duration, events[event_index].get_duration()) != ER_SUCCESS)
            return (ft_nullptr);
        event_index++;
    }
    return (group);
}

int deserialize_event_scheduler(ft_event_scheduler &scheduler, json_group *group)
{
    json_item *count_item = json_find_item(group, "event_count");
    if (!count_item)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    int event_count = ft_atoi(count_item->value);
    int event_index = 0;
    while (event_index < event_count)
    {
        char *event_index_string = cma_itoa(event_index);
        if (!event_index_string)
            return (JSON_MALLOC_FAIL);
        ft_string key_id = "event_";
        key_id += event_index_string;
        key_id += "_id";
        ft_string key_duration = "event_";
        key_duration += event_index_string;
        key_duration += "_duration";
        cma_free(event_index_string);
        json_item *id_item = json_find_item(group, key_id.c_str());
        json_item *duration_item = json_find_item(group, key_duration.c_str());
        if (!id_item || !duration_item)
        {
            ft_errno = GAME_GENERAL_ERROR;
            return (GAME_GENERAL_ERROR);
        }
        ft_event event;
        event.set_id(ft_atoi(id_item->value));
        event.set_duration(ft_atoi(duration_item->value));
        scheduler.schedule_event(event);
        if (scheduler.get_error() != ER_SUCCESS)
            return (scheduler.get_error());
        event_index++;
    }
    return (ER_SUCCESS);
}
