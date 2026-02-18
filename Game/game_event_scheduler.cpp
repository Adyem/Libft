#include "game_event_scheduler.hpp"
#include "../JSon/json.hpp"
#include "../Printf/printf.hpp"
#include "../Template/move.hpp"
#include <cstdio>

static void event_scheduler_profile_reset_struct(t_event_scheduler_profile &profile)
{
    profile.update_count = 0;
    profile.events_processed = 0;
    profile.events_rescheduled = 0;
    profile.max_queue_depth = 0;
    profile.max_ready_batch = 0;
    profile.total_processing_ns = 0;
    profile.last_update_processing_ns = 0;
    profile.last_error_code = FT_ERR_SUCCESS;
    return ;
}

static void event_scheduler_unlock(ft_unique_lock<pt_mutex> &guard)
{
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_event_scheduler::lock_pair(const ft_event_scheduler &first,
        const ft_event_scheduler &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_event_scheduler *ordered_first;
    const ft_event_scheduler *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        first_guard = ft_unique_lock<pt_mutex>(first._mutex);
        second_guard = ft_unique_lock<pt_mutex>();
        if (first_guard.owns_lock() == false)
            return (FT_ERR_INVALID_STATE);
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_event_scheduler *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    first_guard = ft_unique_lock<pt_mutex>(ordered_first->_mutex);
    if (first_guard.owns_lock() == false)
        return (FT_ERR_INVALID_STATE);
    second_guard = ft_unique_lock<pt_mutex>(ordered_second->_mutex);
    if (second_guard.owns_lock() == false)
    {
        event_scheduler_unlock(first_guard);
        return (FT_ERR_INVALID_STATE);
    }
    if (swapped)
    {
        ft_unique_lock<pt_mutex> temporary_guard;

        temporary_guard = ft_move(first_guard);
        first_guard = ft_move(second_guard);
        second_guard = ft_move(temporary_guard);
    }
    return (FT_ERR_SUCCESS);
}

void ft_event_scheduler::reset_profile_locked() const noexcept
{
    event_scheduler_profile_reset_struct(this->_profile);
    this->_profile.last_error_code = this->_error_code;
    return ;
}

void ft_event_scheduler::record_profile_locked(size_t ready_count,
                size_t rescheduled_count,
                size_t queue_depth,
                long long duration_ns) const noexcept
{
    this->_profile.update_count += 1;
    this->_profile.events_processed += static_cast<long long>(ready_count);
    this->_profile.events_rescheduled += static_cast<long long>(rescheduled_count);
    if (queue_depth > this->_profile.max_queue_depth)
        this->_profile.max_queue_depth = queue_depth;
    if (ready_count > this->_profile.max_ready_batch)
        this->_profile.max_ready_batch = ready_count;
    this->_profile.total_processing_ns += duration_ns;
    this->_profile.last_update_processing_ns = duration_ns;
    this->_profile.last_error_code = this->_error_code;
    return ;
}

void ft_event_scheduler::finalize_update(ft_vector<ft_sharedptr<ft_event> > &events,
                size_t ready_count,
                size_t rescheduled_count,
                size_t queue_depth,
                bool profiling_active,
                bool,
                t_high_resolution_time_point) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    this->_ready_cache.clear();
    while (events.size() > 0)
    {
        this->_ready_cache.push_back(events[events.size() - 1]);
        events.pop_back();
    }
    if (profiling_active && this->_profiling_enabled)
        this->record_profile_locked(ready_count, rescheduled_count, queue_depth, 0);
    this->set_error(FT_ERR_SUCCESS);
    event_scheduler_unlock(guard);
    return ;
}

bool ft_event_compare_ptr::operator()(const ft_sharedptr<ft_event> &left,
        const ft_sharedptr<ft_event> &right) const noexcept
{
    const ft_event *left_pointer;
    const ft_event *right_pointer;

    left_pointer = left.get();
    right_pointer = right.get();
    if (left_pointer == ft_nullptr)
        return (false);
    if (right_pointer == ft_nullptr)
        return (true);
    return (left_pointer->get_duration() > right_pointer->get_duration());
}

ft_event_scheduler::ft_event_scheduler() noexcept
    : _events(), _error_code(FT_ERR_SUCCESS), _mutex(), _profiling_enabled(false), _profile(), _ready_cache()
{
    event_scheduler_profile_reset_struct(this->_profile);
    return ;
}

ft_event_scheduler::~ft_event_scheduler()
{
    return ;
}

ft_event_scheduler::ft_event_scheduler(const ft_event_scheduler &other) noexcept
    : _events(), _error_code(FT_ERR_SUCCESS), _mutex(), _profiling_enabled(false), _profile(), _ready_cache()
{
    *this = other;
    return ;
}

ft_event_scheduler &ft_event_scheduler::operator=(const ft_event_scheduler &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_event_scheduler::lock_pair(*this, other,
            this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_profiling_enabled = other._profiling_enabled;
    this->_profile = other._profile;
    this->_events.clear();
    this->_ready_cache.clear();
    this->_error_code = other._error_code;
    event_scheduler_unlock(this_guard);
    event_scheduler_unlock(other_guard);
    return (*this);
}

ft_event_scheduler::ft_event_scheduler(ft_event_scheduler &&other) noexcept
    : _events(), _error_code(FT_ERR_SUCCESS), _mutex(), _profiling_enabled(false), _profile(), _ready_cache()
{
    *this = other;
    return ;
}

ft_event_scheduler &ft_event_scheduler::operator=(ft_event_scheduler &&other) noexcept
{
    *this = other;
    return (*this);
}

void ft_event_scheduler::set_error(int error) const noexcept
{
    this->_error_code = error;
    return ;
}

void ft_event_scheduler::schedule_event(const ft_sharedptr<ft_event> &event) noexcept
{
    if (!event)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return ;
    }
    this->_events.push(event);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_event_scheduler::cancel_event(int id) noexcept
{
    ft_priority_queue<ft_sharedptr<ft_event>, ft_event_compare_ptr> temporary_queue;
    ft_sharedptr<ft_event> current_event;
    bool event_found;

    event_found = false;
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        if (current_event && current_event->get_id() == id)
            event_found = true;
        else
            temporary_queue.push(current_event);
    }
    while (!temporary_queue.empty())
        this->_events.push(temporary_queue.pop());
    if (event_found)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
    return ;
}

void ft_event_scheduler::reschedule_event(int id, int new_duration) noexcept
{
    ft_priority_queue<ft_sharedptr<ft_event>, ft_event_compare_ptr> temporary_queue;
    ft_sharedptr<ft_event> current_event;
    bool event_found;

    event_found = false;
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        if (current_event && current_event->get_id() == id)
        {
            current_event->set_duration(new_duration);
            event_found = true;
        }
        temporary_queue.push(current_event);
    }
    while (!temporary_queue.empty())
        this->_events.push(temporary_queue.pop());
    if (event_found)
        this->set_error(FT_ERR_SUCCESS);
    else
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
    return ;
}

void ft_event_scheduler::update_events(ft_sharedptr<ft_world> &world,
        int ticks, const char *log_file_path,
        ft_string *log_buffer) noexcept
{
    ft_priority_queue<ft_sharedptr<ft_event>, ft_event_compare_ptr> temporary_queue;
    ft_sharedptr<ft_event> current_event;

    if (!world)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return ;
    }
    this->_ready_cache.clear();
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        if (!current_event)
            continue;
        current_event->sub_duration(ticks);
        if (current_event->get_duration() <= 0)
        {
            if (log_file_path)
                (void)log_event_to_file(*current_event, log_file_path);
            if (log_buffer)
                log_event_to_buffer(*current_event, *log_buffer);
            this->_ready_cache.push_back(current_event);
        }
        else
            temporary_queue.push(current_event);
    }
    while (!temporary_queue.empty())
        this->_events.push(temporary_queue.pop());
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_event_scheduler::enable_profiling(bool enabled) noexcept
{
    this->_profiling_enabled = enabled;
    this->reset_profile_locked();
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

bool ft_event_scheduler::profiling_enabled() const noexcept
{
    return (this->_profiling_enabled);
}

void ft_event_scheduler::reset_profile() noexcept
{
    this->reset_profile_locked();
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_event_scheduler::snapshot_profile(t_event_scheduler_profile &out) const noexcept
{
    out = this->_profile;
    return ;
}

void ft_event_scheduler::dump_events(ft_vector<ft_sharedptr<ft_event> > &out) const noexcept
{
    out.clear();
    while (out.size() < this->_ready_cache.size())
    {
        out.push_back(this->_ready_cache[out.size()]);
    }
    return ;
}

size_t ft_event_scheduler::size() const noexcept
{
    return (this->_events.size());
}

void ft_event_scheduler::clear() noexcept
{
    this->_events.clear();
    this->_ready_cache.clear();
    this->set_error(FT_ERR_SUCCESS);
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
    FILE *file;

    file = fopen(file_path, "a");
    if (!file)
        return (FT_ERR_GAME_GENERAL_ERROR);
    if (fprintf(file, "event %d processed\n", event.get_id()) < 0)
    {
        fclose(file);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    fclose(file);
    return (FT_ERR_SUCCESS);
}

void log_event_to_buffer(const ft_event &event, ft_string &buffer) noexcept
{
    char number_buffer[32];

    if (pf_snprintf(number_buffer, sizeof(number_buffer), "%d", event.get_id()) < 0)
        return ;
    buffer += "event ";
    buffer += number_buffer;
    buffer += " processed\n";
    return ;
}

static int add_item_field(json_group *group, const ft_string &key, int value)
{
    json_item *json_item_ptr;

    json_item_ptr = json_create_item(key.c_str(), value);
    if (!json_item_ptr)
    {
        json_free_groups(group);
        return (FT_ERR_NO_MEMORY);
    }
    json_add_item_to_group(group, json_item_ptr);
    return (FT_ERR_SUCCESS);
}

json_group *serialize_event_scheduler(const ft_sharedptr<ft_event_scheduler> &scheduler)
{
    json_group *group;
    json_item *count_item;
    ft_vector<ft_sharedptr<ft_event> > events;
    size_t event_index;
    size_t event_count;

    if (!scheduler)
        return (ft_nullptr);
    group = json_create_json_group("world");
    if (!group)
        return (ft_nullptr);
    count_item = json_create_item("event_count", static_cast<int>(scheduler->size()));
    if (!count_item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, count_item);
    scheduler->dump_events(events);
    event_index = 0;
    event_count = events.size();
    while (event_index < event_count)
    {
        char event_index_buffer[32];
        ft_string key_id;
        ft_string key_duration;

        if (pf_snprintf(event_index_buffer, sizeof(event_index_buffer), "%d", static_cast<int>(event_index)) < 0)
        {
            json_free_groups(group);
            return (ft_nullptr);
        }
        key_id = "event_";
        key_id += event_index_buffer;
        key_id += "_id";
        key_duration = "event_";
        key_duration += event_index_buffer;
        key_duration += "_duration";
        if (add_item_field(group, key_id, events[event_index]->get_id()) != FT_ERR_SUCCESS
            || add_item_field(group, key_duration, events[event_index]->get_duration()) != FT_ERR_SUCCESS)
            return (ft_nullptr);
        event_index += 1;
    }
    return (group);
}

int deserialize_event_scheduler(ft_sharedptr<ft_event_scheduler> &scheduler, json_group *group)
{
    json_item *count_item;
    int event_count;
    int event_index;

    if (!scheduler)
        return (FT_ERR_GAME_GENERAL_ERROR);
    count_item = json_find_item(group, "event_count");
    if (!count_item)
        return (FT_ERR_GAME_GENERAL_ERROR);
    event_count = ft_atoi(count_item->value);
    event_index = 0;
    while (event_index < event_count)
    {
        char event_index_buffer[32];
        ft_string key_id;
        ft_string key_duration;
        json_item *id_item;
        json_item *duration_item;
        ft_sharedptr<ft_event> event;

        if (pf_snprintf(event_index_buffer, sizeof(event_index_buffer), "%d", event_index) < 0)
            return (FT_ERR_NO_MEMORY);
        key_id = "event_";
        key_id += event_index_buffer;
        key_id += "_id";
        key_duration = "event_";
        key_duration += event_index_buffer;
        key_duration += "_duration";
        id_item = json_find_item(group, key_id.c_str());
        duration_item = json_find_item(group, key_duration.c_str());
        if (!id_item || !duration_item)
            return (FT_ERR_GAME_GENERAL_ERROR);
        event = ft_sharedptr<ft_event>(new ft_event());
        event->set_id(ft_atoi(id_item->value));
        event->set_duration(ft_atoi(duration_item->value));
        scheduler->schedule_event(event);
        event_index += 1;
    }
    return (FT_ERR_SUCCESS);
}
