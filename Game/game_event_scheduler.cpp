#include "game_event_scheduler.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../JSon/json.hpp"
#include "../Time/time.hpp"
#include "game_world.hpp"
#include <cstdio>
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void event_scheduler_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void event_scheduler_profile_reset_struct(t_event_scheduler_profile &profile)
{
    profile.update_count = 0;
    profile.events_processed = 0;
    profile.events_rescheduled = 0;
    profile.max_queue_depth = 0;
    profile.max_ready_batch = 0;
    profile.total_processing_ns = 0;
    profile.last_update_processing_ns = 0;
    profile.last_error_code = ER_SUCCESS;
    return ;
}

static void event_scheduler_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
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
        const ft_event_scheduler *temporary;

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
        event_scheduler_sleep_backoff();
    }
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
    long long non_negative_duration;

    non_negative_duration = duration_ns;
    if (non_negative_duration < 0)
        non_negative_duration = 0;
    this->_profile.update_count += 1;
    this->_profile.events_processed += static_cast<long long>(ready_count);
    this->_profile.events_rescheduled += static_cast<long long>(rescheduled_count);
    if (queue_depth > this->_profile.max_queue_depth)
        this->_profile.max_queue_depth = queue_depth;
    if (ready_count > this->_profile.max_ready_batch)
        this->_profile.max_ready_batch = ready_count;
    this->_profile.total_processing_ns += non_negative_duration;
    this->_profile.last_update_processing_ns = non_negative_duration;
    this->_profile.last_error_code = this->_error_code;
    return ;
}

void ft_event_scheduler::finalize_update(ft_vector<ft_sharedptr<ft_event> > &events,
        size_t ready_count,
        size_t rescheduled_count,
        size_t queue_depth,
        bool profiling_active,
        bool start_valid,
        t_high_resolution_time_point start_time) noexcept
{
    int entry_errno;
    bool end_valid;
    t_high_resolution_time_point end_time;
    long long duration_ns;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    end_valid = false;
    duration_ns = 0;
    if (profiling_active && start_valid)
    {
        if (time_high_resolution_now(&end_time))
        {
            duration_ns = time_high_resolution_diff_ns(start_time, end_time);
            end_valid = true;
        }
        else
            profiling_active = false;
    }
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    this->_ready_cache = ft_move(events);
    if (this->_ready_cache.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_ready_cache.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    if (profiling_active && start_valid && end_valid && this->_profiling_enabled)
        this->record_profile_locked(ready_count, rescheduled_count, queue_depth, duration_ns);
    event_scheduler_restore_errno(guard, entry_errno);
    return ;
}

bool ft_event_compare_ptr::operator()(const ft_sharedptr<ft_event> &left,
        const ft_sharedptr<ft_event> &right) const noexcept
{
    const ft_event *left_const_pointer;
    const ft_event *right_const_pointer;
    ft_event *left_pointer;
    ft_event *right_pointer;
    ft_unique_lock<pt_mutex> left_guard;
    ft_unique_lock<pt_mutex> right_guard;
    int entry_errno;
    int lock_error;
    int left_duration;
    int right_duration;
    bool comparison_result;

    left_const_pointer = left.get();
    right_const_pointer = right.get();
    left_pointer = const_cast<ft_event *>(left_const_pointer);
    right_pointer = const_cast<ft_event *>(right_const_pointer);
    if (left_pointer == ft_nullptr && right_pointer == ft_nullptr)
        return (false);
    if (left_pointer == ft_nullptr)
        return (false);
    if (right_pointer == ft_nullptr)
        return (true);
    entry_errno = ft_errno;
    lock_error = ft_event::lock_pair(*left_pointer, *right_pointer,
            left_guard, right_guard);
    if (lock_error != ER_SUCCESS)
    {
        ft_errno = lock_error;
        if (left_guard.owns_lock())
            left_guard.unlock();
        if (right_guard.owns_lock())
            right_guard.unlock();
        return (left_pointer > right_pointer);
    }
    left_duration = left_pointer->_duration;
    right_duration = right_pointer->_duration;
    left_pointer->set_error(ER_SUCCESS);
    right_pointer->set_error(ER_SUCCESS);
    if (left_guard.owns_lock())
        left_guard.unlock();
    if (right_guard.owns_lock())
        right_guard.unlock();
    ft_errno = entry_errno;
    comparison_result = (left_duration > right_duration);
    return (comparison_result);
}

ft_event_scheduler::ft_event_scheduler() noexcept
    : _events(), _error_code(ER_SUCCESS), _mutex(), _profiling_enabled(false), _profile(), _ready_cache()
{
    event_scheduler_profile_reset_struct(this->_profile);
    if (this->_events.get_error() != ER_SUCCESS)
        this->set_error(this->_events.get_error());
    else
        this->set_error(ER_SUCCESS);
    return ;
}

ft_event_scheduler::~ft_event_scheduler()
{
    return ;
}

ft_event_scheduler::ft_event_scheduler(const ft_event_scheduler &other) noexcept
    : _events(), _error_code(ER_SUCCESS), _mutex(), _profiling_enabled(false), _profile(), _ready_cache()
{
    int entry_errno;
    ft_vector<ft_sharedptr<ft_event> > events;
    size_t index;
    size_t count;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    event_scheduler_profile_reset_struct(this->_profile);
    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    other.dump_events(events);
    if (other.get_error() != ER_SUCCESS)
    {
        this->set_error(other.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    this->_profiling_enabled = other._profiling_enabled;
    this->_profile = other._profile;
    if (other._ready_cache.capacity() > 0)
    {
        this->_ready_cache.reserve(other._ready_cache.capacity());
        if (this->_ready_cache.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_ready_cache.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
    }
    index = 0;
    count = events.size();
    while (index < count)
    {
        this->_events.push(events[index]);
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        index += 1;
    }
    this->set_error(other.get_error());
    event_scheduler_restore_errno(guard, entry_errno);
    return ;
}

ft_event_scheduler &ft_event_scheduler::operator=(const ft_event_scheduler &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    ft_vector<ft_sharedptr<ft_event> > events;
    size_t index;
    size_t count;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    other.dump_events(events);
    if (other.get_error() != ER_SUCCESS)
    {
        this->set_error(other.get_error());
        return (*this);
    }
    lock_error = ft_event_scheduler::lock_pair(*this, other,
            this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_profiling_enabled = other._profiling_enabled;
    this->_profile = other._profile;
    this->_ready_cache.clear();
    if (this->_ready_cache.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_ready_cache.get_error());
        event_scheduler_restore_errno(this_guard, entry_errno);
        event_scheduler_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    if (other._ready_cache.capacity() > 0)
    {
        this->_ready_cache.reserve(other._ready_cache.capacity());
        if (this->_ready_cache.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_ready_cache.get_error());
            event_scheduler_restore_errno(this_guard, entry_errno);
            event_scheduler_restore_errno(other_guard, entry_errno);
            return (*this);
        }
    }
    this->_events.clear();
    if (this->_events.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_events.get_error());
        event_scheduler_restore_errno(this_guard, entry_errno);
        event_scheduler_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    index = 0;
    count = events.size();
    while (index < count)
    {
        this->_events.push(events[index]);
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            event_scheduler_restore_errno(this_guard, entry_errno);
            event_scheduler_restore_errno(other_guard, entry_errno);
            return (*this);
        }
        index += 1;
    }
    this->set_error(other.get_error());
    event_scheduler_restore_errno(this_guard, entry_errno);
    event_scheduler_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_event_scheduler::ft_event_scheduler(ft_event_scheduler &&other) noexcept
    : _events(), _error_code(ER_SUCCESS), _mutex(), _profiling_enabled(false), _profile(), _ready_cache()
{
    int entry_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    event_scheduler_profile_reset_struct(this->_profile);
    entry_errno = ft_errno;
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        event_scheduler_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_events = ft_move(other._events);
    if (this->_events.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_events.get_error());
        other.set_error(ER_SUCCESS);
        event_scheduler_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_ready_cache = ft_move(other._ready_cache);
    if (this->_ready_cache.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_ready_cache.get_error());
        other.set_error(ER_SUCCESS);
        event_scheduler_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_error_code = other._error_code;
    this->_profiling_enabled = other._profiling_enabled;
    this->_profile = other._profile;
    this->set_error(this->_error_code);
    other._error_code = ER_SUCCESS;
    other._profiling_enabled = false;
    event_scheduler_profile_reset_struct(other._profile);
    other.set_error(ER_SUCCESS);
    event_scheduler_restore_errno(other_guard, entry_errno);
    return ;
}

ft_event_scheduler &ft_event_scheduler::operator=(ft_event_scheduler &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_event_scheduler::lock_pair(*this, other,
            this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_events = ft_move(other._events);
    if (this->_events.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_events.get_error());
        other.set_error(ER_SUCCESS);
        event_scheduler_restore_errno(this_guard, entry_errno);
        event_scheduler_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_ready_cache = ft_move(other._ready_cache);
    if (this->_ready_cache.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_ready_cache.get_error());
        other.set_error(ER_SUCCESS);
        event_scheduler_restore_errno(this_guard, entry_errno);
        event_scheduler_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_error_code = other._error_code;
    this->_profiling_enabled = other._profiling_enabled;
    this->_profile = other._profile;
    this->set_error(this->_error_code);
    other._error_code = ER_SUCCESS;
    other._profiling_enabled = false;
    event_scheduler_profile_reset_struct(other._profile);
    other.set_error(ER_SUCCESS);
    event_scheduler_restore_errno(this_guard, entry_errno);
    event_scheduler_restore_errno(other_guard, entry_errno);
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
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    if (!event)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    if (event.get_error() != ER_SUCCESS)
    {
        this->set_error(event.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    this->_events.push(event);
    if (this->_events.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_events.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    event_scheduler_restore_errno(guard, entry_errno);
    return ;
}

void ft_event_scheduler::cancel_event(int id) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    ft_priority_queue<ft_sharedptr<ft_event>, ft_event_compare_ptr> temporary_queue;
    ft_sharedptr<ft_event> current_event;
    bool event_found;

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    event_found = false;
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        if (current_event->get_id() != id)
        {
            temporary_queue.push(current_event);
            if (temporary_queue.get_error() != ER_SUCCESS)
            {
                this->set_error(temporary_queue.get_error());
                event_scheduler_restore_errno(guard, entry_errno);
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
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        this->_events.push(temporary_event);
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
    }
    if (!event_found)
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
    else
        this->set_error(ER_SUCCESS);
    event_scheduler_restore_errno(guard, entry_errno);
    return ;
}

void ft_event_scheduler::reschedule_event(int id, int new_duration) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    ft_priority_queue<ft_sharedptr<ft_event>, ft_event_compare_ptr> temporary_queue;
    ft_sharedptr<ft_event> current_event;
    bool event_found;

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    event_found = false;
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        if (current_event->get_id() == id)
        {
            current_event->set_duration(new_duration);
            if (current_event->get_error() != ER_SUCCESS)
            {
                this->set_error(current_event->get_error());
                event_scheduler_restore_errno(guard, entry_errno);
                return ;
            }
            event_found = true;
        }
        temporary_queue.push(current_event);
        if (temporary_queue.get_error() != ER_SUCCESS)
        {
            this->set_error(temporary_queue.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
    }
    while (!temporary_queue.empty())
    {
        ft_sharedptr<ft_event> temporary_event = temporary_queue.pop();
        if (temporary_queue.get_error() != ER_SUCCESS)
        {
            this->set_error(temporary_queue.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        this->_events.push(temporary_event);
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
    }
    if (!event_found)
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
    else
        this->set_error(ER_SUCCESS);
    event_scheduler_restore_errno(guard, entry_errno);
    return ;
}

void ft_event_scheduler::update_events(ft_sharedptr<ft_world> &world,
        int ticks, const char *log_file_path,
        ft_string *log_buffer) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    ft_priority_queue<ft_sharedptr<ft_event>, ft_event_compare_ptr> temporary_queue;
    ft_sharedptr<ft_event> current_event;
    ft_vector<ft_sharedptr<ft_event> > events_to_process;
    ft_vector<ft_sharedptr<ft_event> > &ready_cache = this->_ready_cache;
    size_t ready_index;
    size_t ready_count;
    size_t rescheduled_count;
    size_t queue_depth;
    bool profiling_active;
    bool start_valid;
    t_high_resolution_time_point start_time;

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    if (!world)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    ready_cache.clear();
    if (ready_cache.get_error() != ER_SUCCESS)
    {
        this->set_error(ready_cache.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    profiling_active = this->_profiling_enabled;
    start_valid = false;
    if (profiling_active)
    {
        if (time_high_resolution_now(&start_time))
            start_valid = true;
        else
            profiling_active = false;
    }
    rescheduled_count = 0;
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        if (current_event.get_error() != ER_SUCCESS)
        {
            this->set_error(current_event.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        current_event->sub_duration(ticks);
        if (current_event->get_error() != ER_SUCCESS)
        {
            this->set_error(current_event->get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        if (current_event->get_duration() <= 0)
        {
            ready_cache.push_back(current_event);
            if (ready_cache.get_error() != ER_SUCCESS)
            {
                this->set_error(ready_cache.get_error());
                event_scheduler_restore_errno(guard, entry_errno);
                return ;
            }
        }
        else
        {
            temporary_queue.push(current_event);
            if (temporary_queue.get_error() != ER_SUCCESS)
            {
                this->set_error(temporary_queue.get_error());
                event_scheduler_restore_errno(guard, entry_errno);
                return ;
            }
        }
    }
    while (!temporary_queue.empty())
    {
        ft_sharedptr<ft_event> temporary_event = temporary_queue.pop();
        if (temporary_queue.get_error() != ER_SUCCESS)
        {
            this->set_error(temporary_queue.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        if (temporary_event.get_error() != ER_SUCCESS)
        {
            this->set_error(temporary_event.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        this->_events.push(temporary_event);
        if (this->_events.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_events.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        rescheduled_count += 1;
    }
    queue_depth = this->_events.size();
    events_to_process = ft_move(ready_cache);
    ready_count = events_to_process.size();
    this->set_error(ER_SUCCESS);
    event_scheduler_restore_errno(guard, entry_errno);
    ready_index = 0;
    while (ready_index < ready_count)
    {
        ft_sharedptr<ft_event> &processed_event = events_to_process[ready_index];
        const ft_function<void(ft_world&, ft_event&)> &callback = processed_event->get_callback();
        int processed_error;

        processed_error = processed_event->get_error();
        if (processed_error != ER_SUCCESS)
        {
            int callback_entry_errno;

            callback_entry_errno = ft_errno;
            ft_unique_lock<pt_mutex> error_guard(this->_mutex);

            if (error_guard.get_error() == ER_SUCCESS)
            {
                this->set_error(processed_error);
                event_scheduler_restore_errno(error_guard, callback_entry_errno);
            }
            else
            {
                this->set_error(error_guard.get_error());
                event_scheduler_restore_errno(error_guard, callback_entry_errno);
            }
            this->finalize_update(events_to_process, ready_count, rescheduled_count, queue_depth, profiling_active, start_valid, start_time);
            return ;
        }
        if (callback)
            callback(*world, *processed_event);
        if (log_file_path)
        {
            int log_result;

            log_result = log_event_to_file(*processed_event, log_file_path);
            if (log_result != ER_SUCCESS)
            {
                int callback_entry_errno;

                callback_entry_errno = ft_errno;
                ft_unique_lock<pt_mutex> error_guard(this->_mutex);

                if (error_guard.get_error() == ER_SUCCESS)
                {
                    this->set_error(log_result);
                    event_scheduler_restore_errno(error_guard, callback_entry_errno);
                }
                else
                {
                    this->set_error(error_guard.get_error());
                    event_scheduler_restore_errno(error_guard, callback_entry_errno);
                }
                this->finalize_update(events_to_process, ready_count, rescheduled_count, queue_depth, profiling_active, start_valid, start_time);
                return ;
            }
        }
        if (log_buffer)
        {
            log_event_to_buffer(*processed_event, *log_buffer);
            if (ft_errno != ER_SUCCESS)
            {
                int callback_entry_errno;

                callback_entry_errno = ft_errno;
                ft_unique_lock<pt_mutex> error_guard(this->_mutex);

                if (error_guard.get_error() == ER_SUCCESS)
                {
                    this->set_error(callback_entry_errno);
                    event_scheduler_restore_errno(error_guard, callback_entry_errno);
                }
                else
                {
                    this->set_error(error_guard.get_error());
                    event_scheduler_restore_errno(error_guard, callback_entry_errno);
                }
                this->finalize_update(events_to_process, ready_count, rescheduled_count, queue_depth, profiling_active, start_valid, start_time);
                return ;
            }
        }
        ready_index += 1;
    }
    this->finalize_update(events_to_process, ready_count, rescheduled_count, queue_depth, profiling_active, start_valid, start_time);
    return ;
}

void ft_event_scheduler::enable_profiling(bool enabled) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    this->_profiling_enabled = enabled;
    this->reset_profile_locked();
    this->set_error(ER_SUCCESS);
    event_scheduler_restore_errno(guard, entry_errno);
    return ;
}

bool ft_event_scheduler::profiling_enabled() const noexcept
{
    int entry_errno;
    bool enabled;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_event_scheduler *>(this)->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return (false);
    }
    enabled = this->_profiling_enabled;
    const_cast<ft_event_scheduler *>(this)->set_error(this->_error_code);
    event_scheduler_restore_errno(guard, entry_errno);
    return (enabled);
}

void ft_event_scheduler::reset_profile() noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    this->reset_profile_locked();
    this->set_error(ER_SUCCESS);
    event_scheduler_restore_errno(guard, entry_errno);
    return ;
}

void ft_event_scheduler::snapshot_profile(t_event_scheduler_profile &out) const noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        event_scheduler_profile_reset_struct(out);
        const_cast<ft_event_scheduler *>(this)->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    out = this->_profile;
    const_cast<ft_event_scheduler *>(this)->set_error(this->_error_code);
    event_scheduler_restore_errno(guard, entry_errno);
    return ;
}

void ft_event_scheduler::dump_events(ft_vector<ft_sharedptr<ft_event> > &out) const noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    ft_priority_queue<ft_sharedptr<ft_event>, ft_event_compare_ptr> temporary_queue;
    ft_sharedptr<ft_event> current_event;

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_event_scheduler *>(this)->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    const_cast<ft_event_scheduler *>(this)->set_error(ER_SUCCESS);
    while (!this->_events.empty())
    {
        current_event = this->_events.pop();
        if (this->_events.get_error() != ER_SUCCESS)
        {
            const_cast<ft_event_scheduler *>(this)->set_error(this->_events.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        out.push_back(current_event);
        if (out.get_error() != ER_SUCCESS)
        {
            const_cast<ft_event_scheduler *>(this)->set_error(out.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        temporary_queue.push(current_event);
        if (temporary_queue.get_error() != ER_SUCCESS)
        {
            const_cast<ft_event_scheduler *>(this)->set_error(temporary_queue.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
    }
    while (!temporary_queue.empty())
    {
        this->_events.push(temporary_queue.pop());
        if (temporary_queue.get_error() != ER_SUCCESS)
        {
            const_cast<ft_event_scheduler *>(this)->set_error(temporary_queue.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
        if (this->_events.get_error() != ER_SUCCESS)
        {
            const_cast<ft_event_scheduler *>(this)->set_error(this->_events.get_error());
            event_scheduler_restore_errno(guard, entry_errno);
            return ;
        }
    }
    const_cast<ft_event_scheduler *>(this)->set_error(ER_SUCCESS);
    event_scheduler_restore_errno(guard, entry_errno);
    return ;
}

size_t ft_event_scheduler::size() const noexcept
{
    int entry_errno;
    size_t event_count;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_event_scheduler *>(this)->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return (0);
    }
    event_count = this->_events.size();
    const_cast<ft_event_scheduler *>(this)->set_error(ER_SUCCESS);
    event_scheduler_restore_errno(guard, entry_errno);
    return (event_count);
}

void ft_event_scheduler::clear() noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    this->_events.clear();
    if (this->_events.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_events.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    event_scheduler_restore_errno(guard, entry_errno);
    return ;
}

int ft_event_scheduler::get_error() const noexcept
{
    int entry_errno;
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_event_scheduler *>(this)->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_event_scheduler *>(this)->set_error(error_code);
    event_scheduler_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_event_scheduler::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_event_scheduler *>(this)->set_error(guard.get_error());
        event_scheduler_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_event_scheduler *>(this)->set_error(error_code);
    event_scheduler_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

int log_event_to_file(const ft_event &event, const char *file_path) noexcept
{
    FILE *file = fopen(file_path, "a");
    if (!file)
    {
        ft_errno = FT_ERR_GAME_GENERAL_ERROR;
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    if (fprintf(file, "event %d processed\n", event.get_id()) < 0)
    {
        fclose(file);
        ft_errno = FT_ERR_GAME_GENERAL_ERROR;
        return (FT_ERR_GAME_GENERAL_ERROR);
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
        ft_errno = FT_ERR_NO_MEMORY;
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
        ft_errno = FT_ERR_NO_MEMORY;
        return (FT_ERR_NO_MEMORY);
    }
    json_add_item_to_group(group, json_item_ptr);
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

json_group *serialize_event_scheduler(const ft_sharedptr<ft_event_scheduler> &scheduler)
{
    if (!scheduler)
    {
        ft_errno = FT_ERR_GAME_GENERAL_ERROR;
        return (ft_nullptr);
    }
    json_group *group = json_create_json_group("world");
    if (!group)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    json_item *count_item = json_create_item("event_count", static_cast<int>(scheduler->size()));
    if (!count_item)
    {
        json_free_groups(group);
        ft_errno = FT_ERR_NO_MEMORY;
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
            ft_errno = FT_ERR_NO_MEMORY;
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
        ft_errno = FT_ERR_GAME_GENERAL_ERROR;
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    json_item *count_item = json_find_item(group, "event_count");
    if (!count_item)
    {
        ft_errno = FT_ERR_GAME_GENERAL_ERROR;
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    int event_count = ft_atoi(count_item->value);
    int event_index = 0;
    while (event_index < event_count)
    {
        char *event_index_string = cma_itoa(event_index);
        if (!event_index_string)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            return (FT_ERR_NO_MEMORY);
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
            ft_errno = FT_ERR_GAME_GENERAL_ERROR;
            return (FT_ERR_GAME_GENERAL_ERROR);
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
