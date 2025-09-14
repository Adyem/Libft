#ifndef GAME_EVENT_SCHEDULER_HPP
# define GAME_EVENT_SCHEDULER_HPP

#include "game_event.hpp"
#include "../Template/priority_queue.hpp"
#include "../Template/vector.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

struct json_group;

struct ft_event_compare
{
    bool operator()(const ft_event &left, const ft_event &right) const noexcept;
};

class ft_event_scheduler
{
    private:
        mutable ft_priority_queue<ft_event, ft_event_compare> _events;
        mutable int _error_code;

        void set_error(int error) const noexcept;

    public:
        ft_event_scheduler() noexcept;
        ~ft_event_scheduler();

        void schedule_event(const ft_event &event) noexcept;
        void update_events(int ticks, const char *log_file_path = ft_nullptr, ft_string *log_buffer = ft_nullptr) noexcept;

        void dump_events(ft_vector<ft_event> &out) const noexcept;
        size_t size() const noexcept;
        void clear() noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

int log_event_to_file(const ft_event &event, const char *file_path) noexcept;
void log_event_to_buffer(const ft_event &event, ft_string &buffer) noexcept;

json_group *serialize_event_scheduler(const ft_event_scheduler &scheduler);
int deserialize_event_scheduler(ft_event_scheduler &scheduler, json_group *group);

#endif
