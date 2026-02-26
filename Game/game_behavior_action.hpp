#ifndef FT_BEHAVIOR_ACTION_HPP
# define FT_BEHAVIOR_ACTION_HPP

#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"

class ft_behavior_action
{
    private:
        int              _action_id;
        double           _weight;
        double           _cooldown_seconds;
        static thread_local int _last_error;
        mutable pt_recursive_mutex *_mutex;

        void set_error(int error_code) const noexcept;
        int lock_internal(bool *lock_acquired) const noexcept;
        void unlock_internal(bool lock_acquired) const noexcept;
        static int lock_pair(const ft_behavior_action &first, const ft_behavior_action &second,
                bool *first_locked,
                bool *second_locked);

    public:
        ft_behavior_action() noexcept;
        virtual ~ft_behavior_action() noexcept;
        ft_behavior_action(const ft_behavior_action &other) noexcept = delete;
        ft_behavior_action &operator=(const ft_behavior_action &other) noexcept = delete;
        ft_behavior_action(ft_behavior_action &&other) noexcept = delete;
        ft_behavior_action &operator=(ft_behavior_action &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(int action_id, double weight, double cooldown_seconds) noexcept;
        int initialize(const ft_behavior_action &other) noexcept;
        int initialize(ft_behavior_action &&other) noexcept;

        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;

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
