#ifndef GAME_EVENT_SCHEDULER_HPP
# define GAME_EVENT_SCHEDULER_HPP

#include "game_event.hpp"
#include "../Template/priority_queue.hpp"
#include "../Template/vector.hpp"
#include "../Template/shared_ptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include "../Time/time.hpp"
#include <cstddef>

struct json_group;
class ft_world;

typedef struct s_event_scheduler_profile
{
    long long update_count;
    long long events_processed;
    long long events_rescheduled;
    size_t max_queue_depth;
    size_t max_ready_batch;
    long long total_processing_ns;
    long long last_update_processing_ns;
    int last_error_code;
}   t_event_scheduler_profile;

struct ft_event_compare_ptr
{
    bool operator()(const ft_sharedptr<ft_event> &left, const ft_sharedptr<ft_event> &right) const noexcept;
};

class ft_event_scheduler
{
    private:
        mutable ft_priority_queue<ft_sharedptr<ft_event>, ft_event_compare_ptr> _events;
        static thread_local int _last_error;
        mutable pt_recursive_mutex *_mutex;
        mutable bool _profiling_enabled;
        mutable t_event_scheduler_profile _profile;
        mutable ft_vector<ft_sharedptr<ft_event> > _ready_cache;

        void set_error(int error) const noexcept;
        int lock_internal(bool *lock_acquired) const noexcept;
        void unlock_internal(bool lock_acquired) const noexcept;
        void reset_profile_locked() const noexcept;
        void record_profile_locked(size_t ready_count,
                size_t rescheduled_count,
                size_t queue_depth,
                long long duration_ns) const noexcept;
        void finalize_update(ft_vector<ft_sharedptr<ft_event> > &events,
                size_t ready_count,
                size_t rescheduled_count,
                size_t queue_depth,
                bool profiling_active,
                bool start_valid,
                t_high_resolution_time_point start_time) noexcept;

    public:
        ft_event_scheduler() noexcept;
        ~ft_event_scheduler();
        ft_event_scheduler(const ft_event_scheduler &other) noexcept = delete;
        ft_event_scheduler &operator=(const ft_event_scheduler &other) noexcept = delete;
        ft_event_scheduler(ft_event_scheduler &&other) noexcept = delete;
        ft_event_scheduler &operator=(ft_event_scheduler &&other) noexcept = delete;

        void schedule_event(const ft_sharedptr<ft_event> &event) noexcept;
        void cancel_event(int id) noexcept;
        void reschedule_event(int id, int new_duration) noexcept;
        void update_events(ft_sharedptr<ft_world> &world, int ticks, const char *log_file_path = ft_nullptr, ft_string *log_buffer = ft_nullptr) noexcept;

        void enable_profiling(bool enabled) noexcept;
        bool profiling_enabled() const noexcept;
        void reset_profile() noexcept;
        void snapshot_profile(t_event_scheduler_profile &out) const noexcept;

        void dump_events(ft_vector<ft_sharedptr<ft_event> > &out) const noexcept;
        size_t size() const noexcept;
        void clear() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

int log_event_to_file(const ft_event &event, const char *file_path) noexcept;
void log_event_to_buffer(const ft_event &event, ft_string &buffer) noexcept;

json_group *serialize_event_scheduler(const ft_sharedptr<ft_event_scheduler> &scheduler);
int deserialize_event_scheduler(ft_sharedptr<ft_event_scheduler> &scheduler, json_group *group);

#endif
