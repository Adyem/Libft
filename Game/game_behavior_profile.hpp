#ifndef FT_BEHAVIOR_PROFILE_HPP
# define FT_BEHAVIOR_PROFILE_HPP

#include "../Errno/errno.hpp"
#include "../Template/vector.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include "game_behavior_action.hpp"
#include <stdint.h>

class ft_behavior_profile
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t                           _profile_id;
        double                        _aggression_weight;
        double                        _caution_weight;
        ft_vector<ft_behavior_action> _actions;
        pt_recursive_mutex                     *_mutex;
        static thread_local int32_t _last_error;
        uint8_t                       _initialised_state;


        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        int32_t unlock_internal(ft_bool lock_acquired) const noexcept;
        static int32_t set_error(int32_t error_code) noexcept;

        friend class ft_behavior_table;

    public:
        ft_behavior_profile() noexcept;
        virtual ~ft_behavior_profile() noexcept;
        ft_behavior_profile(const ft_behavior_profile &other) noexcept;
        ft_behavior_profile &operator=(const ft_behavior_profile &other) = delete;
        ft_behavior_profile(ft_behavior_profile &&other) noexcept;
        ft_behavior_profile &operator=(ft_behavior_profile &&other) = delete;

        int32_t initialize() noexcept;
        int32_t initialize(const ft_behavior_profile &other) noexcept;
        int32_t initialize(ft_behavior_profile &&other) noexcept;
        int32_t move(ft_behavior_profile &other) noexcept;
        int32_t initialize(int32_t profile_id, double aggression_weight,
                double caution_weight,
                const ft_vector<ft_behavior_action> &actions) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t lock(ft_bool *lock_acquired) const noexcept;
        void unlock(ft_bool lock_acquired) const noexcept;

        int32_t get_profile_id() const noexcept;
        void set_profile_id(int32_t profile_id) noexcept;

        double get_aggression_weight() const noexcept;
        void set_aggression_weight(double aggression_weight) noexcept;

        double get_caution_weight() const noexcept;
        void set_caution_weight(double caution_weight) noexcept;

        ft_vector<ft_behavior_action> &get_actions() noexcept;
        const ft_vector<ft_behavior_action> &get_actions() const noexcept;
        void set_actions(const ft_vector<ft_behavior_action> &actions) noexcept;

        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;

};

#endif
