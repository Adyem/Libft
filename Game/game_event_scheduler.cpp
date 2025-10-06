#include "game_event_scheduler.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../JSon/json.hpp"
#include "game_world.hpp"
#include <cstdio>
#include <utility>

bool ft_event_compare_ptr::operator()(const ft_sharedptr<ft_event> &left, const ft_sharedptr<ft_event> &right) const noexcept
{
    return (left->get_duration() > right->get_duration());
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

ft_event_scheduler::ft_event_scheduler(const ft_event_scheduler &other) noexcept
    : _events(), _error_code(other._error_code)
{
    ft_vector<ft_sharedptr<ft_event> > events;
    other.dump_events(events);
    size_t index = 0;
    while (index < events.size())
    {
        this->_events.push(events[index]);
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            return ;
        }
        ++index;
    }
    return ;
}

ft_event_scheduler &ft_event_scheduler::operator=(const ft_event_scheduler &other) noexcept
{
    if (this != &other)
    {
        this->_events.clear();
        ft_vector<ft_sharedptr<ft_event> > events;
        other.dump_events(events);
        size_t index = 0;
        while (index < events.size())
        {
            this->_events.push(events[index]);
            if (this->_events.get_error() != ER_SUCCESS)
            {
                this->set_error(this->_events.get_error());
                return (*this);
            }
            ++index;
        }
        this->_error_code = other._error_code;
    }
    return (*this);
}

ft_event_scheduler::ft_event_scheduler(ft_event_scheduler &&other) noexcept
    : _events(std::move(other._events)), _error_code(other._error_code)
{
    if (this->_events.get_error() != ER_SUCCESS)
        this->set_error(this->_events.get_error());
    other._error_code = ER_SUCCESS;
    return ;
}

ft_event_scheduler &ft_event_scheduler::operator=(ft_event_scheduler &&other) noexcept
{
    if (this != &other)
    {
        this->_events = std::move(other._events);
        this->_error_code = other._error_code;
        if (this->_events.get_error() != ER_SUCCESS)
            this->set_error(this->_events.get_error());
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

void ft_event_scheduler::set_error(int error) const noexcept
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

void ft_event_scheduler::schedule_event(const ft_sharedptr<ft_event> &event) noexcept
{
    this->set_error(ER_SUCCESS);
    if (!event)
    {
        this->set_error(GAME_GENERAL_ERROR);
        return ;
    }
    if (event.get_error() != ER_SUCCESS)
    {
        this->set_error(event.get_error());
        return ;
    }
    this->_events.push(event);
    if (this->_events.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_events.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_event_scheduler::cancel_event(int id) noexcept
{
    this->set_error(ER_SUCCESS);
    ft_priority_queue<ft_sharedptr<ft_event>, ft_event_compare_ptr> temporary_queue;
    ft_sharedptr<ft_event> current_event;
    bool event_found = false;
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            return ;
        }
        if (current_event->get_id() != id)
        {
            temporary_queue.push(current_event);
            if (temporary_queue.get_error() != ER_SUCCESS)
            {
                this->set_error(temporary_queue.get_error());
                return ;
            }
        }
        else
            event_found = true;
    }
    while (!temporary_queue.empty())
    {
        ft_sharedptr<ft_event> temporary_event = temporary_queue.pop();
        if (temporary_queue.get_error() != ER_SUCCESS)
        {
            this->set_error(temporary_queue.get_error());
            return ;
        }
        this->_events.push(temporary_event);
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            return ;
        }
    }
    if (!event_found)
        this->set_error(GAME_GENERAL_ERROR);
    else
        this->set_error(ER_SUCCESS);
    return ;
}

void ft_event_scheduler::reschedule_event(int id, int new_duration) noexcept
{
    this->set_error(ER_SUCCESS);
    ft_priority_queue<ft_sharedptr<ft_event>, ft_event_compare_ptr> temporary_queue;
    ft_sharedptr<ft_event> current_event;
    bool event_found = false;
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            return ;
        }
        if (current_event->get_id() == id)
        {
            current_event->set_duration(new_duration);
            if (current_event->get_error() != ER_SUCCESS)
            {
                this->set_error(current_event->get_error());
                return ;
            }
            event_found = true;
        }
        temporary_queue.push(current_event);
        if (temporary_queue.get_error() != ER_SUCCESS)
        {
            this->set_error(temporary_queue.get_error());
            return ;
        }
    }
    while (!temporary_queue.empty())
    {
        ft_sharedptr<ft_event> temporary_event = temporary_queue.pop();
        if (temporary_queue.get_error() != ER_SUCCESS)
        {
            this->set_error(temporary_queue.get_error());
            return ;
        }
        this->_events.push(temporary_event);
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            return ;
        }
    }
    if (!event_found)
        this->set_error(GAME_GENERAL_ERROR);
    else
        this->set_error(ER_SUCCESS);
    return ;
}

void ft_event_scheduler::update_events(ft_sharedptr<ft_world> &world, int ticks, const char *log_file_path, ft_string *log_buffer) noexcept
{
    this->set_error(ER_SUCCESS);
    if (!world)
    {
        this->set_error(GAME_GENERAL_ERROR);
        return ;
    }
    ft_priority_queue<ft_sharedptr<ft_event>, ft_event_compare_ptr> temp;
    ft_sharedptr<ft_event> current_event;
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            return ;
        }
        if (current_event.get_error() != ER_SUCCESS)
        {
            this->set_error(current_event.get_error());
            return ;
        }
        current_event->sub_duration(ticks);
        if (current_event->get_error() != ER_SUCCESS)
        {
            this->set_error(current_event->get_error());
            return ;
        }
        if (current_event->get_duration() <= 0)
        {
            const ft_function<void(ft_world&, ft_event&)> &callback = current_event->get_callback();
            if (callback)
                callback(*world, *current_event);
            if (log_file_path)
            {
                int log_result = log_event_to_file(*current_event, log_file_path);
                if (log_result != ER_SUCCESS)
                {
                    this->set_error(log_result);
                    return ;
                }
            }
            if (log_buffer)
            {
                log_event_to_buffer(*current_event, *log_buffer);
                if (ft_errno != ER_SUCCESS)
                {
                    this->set_error(ft_errno);
                    return ;
                }
            }
        }
        else
        {
            temp.push(current_event);
            if (temp.get_error() != ER_SUCCESS)
            {
                this->set_error(temp.get_error());
                return ;
            }
        }
    }
    while (!temp.empty())
    {
        ft_sharedptr<ft_event> temp_event = temp.pop();
        if (temp.get_error() != ER_SUCCESS)
        {
            this->set_error(temp.get_error());
            return ;
        }
        if (temp_event.get_error() != ER_SUCCESS)
        {
            this->set_error(temp_event.get_error());
            return ;
        }
        this->_events.push(temp_event);
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            return ;
        }
    }
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_event_scheduler::dump_events(ft_vector<ft_sharedptr<ft_event> > &out) const noexcept
{
    this->set_error(ER_SUCCESS);
    ft_priority_queue<ft_sharedptr<ft_event>, ft_event_compare_ptr> temp;
    ft_sharedptr<ft_event> current_event;
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            return ;
        }
        out.push_back(current_event);
        if (out.get_error() != ER_SUCCESS)
        {
            this->set_error(out.get_error());
            return ;
        }
        temp.push(current_event);
        if (temp.get_error() != ER_SUCCESS)
        {
            this->set_error(temp.get_error());
            return ;
        }
    }
    while (!temp.empty())
    {
        this->_events.push(temp.pop());
        if (temp.get_error() != ER_SUCCESS)
        {
            this->set_error(temp.get_error());
            return ;
        }
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            return ;
        }
    }
    this->set_error(ER_SUCCESS);
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
    {
        this->set_error(this->_events.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
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
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    if (fprintf(file, "event %d processed\n", event.get_id()) < 0)
    {
        fclose(file);
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    fclose(file);
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

void log_event_to_buffer(const ft_event &event, ft_string &buffer) noexcept
{
    char *id_string = cma_itoa(event.get_id());
    if (!id_string)
    {
        ft_errno = FT_EALLOC;
        return ;
    }
    buffer += "event ";
    buffer += id_string;
    buffer += " processed\n";
    cma_free(id_string);
    ft_errno = ER_SUCCESS;
    return ;
}

static int add_item_field(json_group *group, const ft_string &key, int value)
{
    json_item *json_item_ptr = json_create_item(key.c_str(), value);
    if (!json_item_ptr)
    {
        json_free_groups(group);
        ft_errno = JSON_MALLOC_FAIL;
        return (JSON_MALLOC_FAIL);
    }
    json_add_item_to_group(group, json_item_ptr);
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

json_group *serialize_event_scheduler(const ft_sharedptr<ft_event_scheduler> &scheduler)
{
    if (!scheduler)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (ft_nullptr);
    }
    json_group *group = json_create_json_group("world");
    if (!group)
    {
        ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
    json_item *count_item = json_create_item("event_count", static_cast<int>(scheduler->size()));
    if (!count_item)
    {
        json_free_groups(group);
        ft_errno = JSON_MALLOC_FAIL;
        return (ft_nullptr);
    }
    json_add_item_to_group(group, count_item);
    ft_vector<ft_sharedptr<ft_event> > events;
    scheduler->dump_events(events);
    if (scheduler->get_error() != ER_SUCCESS)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    size_t event_index = 0;
    size_t event_count = events.size();
    while (event_index < event_count)
    {
        char *event_index_string = cma_itoa(static_cast<int>(event_index));
        if (!event_index_string)
        {
            json_free_groups(group);
            ft_errno = JSON_MALLOC_FAIL;
            return (ft_nullptr);
        }
        ft_string key_id = "event_";
        key_id += event_index_string;
        key_id += "_id";
        ft_string key_duration = "event_";
        key_duration += event_index_string;
        key_duration += "_duration";
        cma_free(event_index_string);
        if (add_item_field(group, key_id, events[event_index]->get_id()) != ER_SUCCESS ||
            add_item_field(group, key_duration, events[event_index]->get_duration()) != ER_SUCCESS)
            return (ft_nullptr);
        event_index++;
    }
    ft_errno = ER_SUCCESS;
    return (group);
}

int deserialize_event_scheduler(ft_sharedptr<ft_event_scheduler> &scheduler, json_group *group)
{
    ft_errno = ER_SUCCESS;
    if (!scheduler)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
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
        {
            ft_errno = JSON_MALLOC_FAIL;
            return (JSON_MALLOC_FAIL);
        }
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
        ft_sharedptr<ft_event> event(new ft_event());
        if (event.get_error() != ER_SUCCESS)
        {
            ft_errno = event.get_error();
            return (event.get_error());
        }
        event->set_id(ft_atoi(id_item->value));
        event->set_duration(ft_atoi(duration_item->value));
        if (event->get_error() != ER_SUCCESS)
        {
            ft_errno = event->get_error();
            return (event->get_error());
        }
        scheduler->schedule_event(event);
        if (scheduler->get_error() != ER_SUCCESS)
            return (scheduler->get_error());
        event_index++;
    }
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}
