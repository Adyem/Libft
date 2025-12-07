#ifndef FT_BEHAVIOR_ACTION_HPP
# define FT_BEHAVIOR_ACTION_HPP

#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_behavior_action
{
    private:
        int              _action_id;
        double           _weight;
        double           _cooldown_seconds;
        mutable int      _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_behavior_action &first, const ft_behavior_action &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_behavior_action() noexcept;
        ft_behavior_action(int action_id, double weight, double cooldown_seconds) noexcept;
        virtual ~ft_behavior_action() = default;
        ft_behavior_action(const ft_behavior_action &other) noexcept;
        ft_behavior_action &operator=(const ft_behavior_action &other) noexcept;
        ft_behavior_action(ft_behavior_action &&other) noexcept;
        ft_behavior_action &operator=(ft_behavior_action &&other) noexcept;

        int get_action_id() const noexcept;
        void set_action_id(int action_id) noexcept;

        double get_weight() const noexcept;
        void set_weight(double weight) noexcept;

        double get_cooldown_seconds() const noexcept;
        void set_cooldown_seconds(double cooldown_seconds) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
