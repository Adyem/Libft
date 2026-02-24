#ifndef FT_BEHAVIOR_PROFILE_HPP
# define FT_BEHAVIOR_PROFILE_HPP

#include "../Errno/errno.hpp"
#include "../Template/vector.hpp"
#include "../PThread/mutex.hpp"
#include "game_behavior_action.hpp"
#include <stdint.h>

class ft_behavior_profile
{
    private:
        int                           _profile_id;
        double                        _aggression_weight;
        double                        _caution_weight;
        ft_vector<ft_behavior_action> _actions;
        pt_recursive_mutex                     *_mutex;
        uint8_t                       _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

        friend class ft_behavior_table;

    public:
        ft_behavior_profile() noexcept;
        virtual ~ft_behavior_profile() noexcept;
        ft_behavior_profile(const ft_behavior_profile &other) = delete;
        ft_behavior_profile &operator=(const ft_behavior_profile &other) = delete;
        ft_behavior_profile(ft_behavior_profile &&other) = delete;
        ft_behavior_profile &operator=(ft_behavior_profile &&other) = delete;

        int initialize() noexcept;
        int initialize(const ft_behavior_profile &other) noexcept;
        int initialize(ft_behavior_profile &&other) noexcept;
        int initialize(int profile_id, double aggression_weight,
                double caution_weight,
                const ft_vector<ft_behavior_action> &actions) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int get_profile_id() const noexcept;
        void set_profile_id(int profile_id) noexcept;

        double get_aggression_weight() const noexcept;
        void set_aggression_weight(double aggression_weight) noexcept;

        double get_caution_weight() const noexcept;
        void set_caution_weight(double caution_weight) noexcept;

        ft_vector<ft_behavior_action> &get_actions() noexcept;
        const ft_vector<ft_behavior_action> &get_actions() const noexcept;
        void set_actions(const ft_vector<ft_behavior_action> &actions) noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
