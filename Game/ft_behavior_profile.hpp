#ifndef FT_BEHAVIOR_PROFILE_HPP
# define FT_BEHAVIOR_PROFILE_HPP

#include "../Errno/errno.hpp"
#include "../Template/vector.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "ft_behavior_action.hpp"

class ft_behavior_profile
{
    private:
        int                             _profile_id;
        double                          _aggression_weight;
        double                          _caution_weight;
        ft_vector<ft_behavior_action>   _actions;
        mutable int                     _error_code;
        mutable pt_mutex                _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_behavior_profile &first, const ft_behavior_profile &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);
        static int copy_without_lock(ft_behavior_profile &destination, const ft_behavior_profile &source) noexcept;

        friend class ft_behavior_table;

    public:
        ft_behavior_profile() noexcept;
        ft_behavior_profile(int profile_id, double aggression_weight, double caution_weight,
                const ft_vector<ft_behavior_action> &actions) noexcept;
        virtual ~ft_behavior_profile() = default;
        ft_behavior_profile(const ft_behavior_profile &other) noexcept;
        ft_behavior_profile &operator=(const ft_behavior_profile &other) noexcept;
        ft_behavior_profile(ft_behavior_profile &&other) noexcept;
        ft_behavior_profile &operator=(ft_behavior_profile &&other) noexcept;

        int get_profile_id() const noexcept;
        void set_profile_id(int profile_id) noexcept;

        double get_aggression_weight() const noexcept;
        void set_aggression_weight(double aggression_weight) noexcept;

        double get_caution_weight() const noexcept;
        void set_caution_weight(double caution_weight) noexcept;

        ft_vector<ft_behavior_action> &get_actions() noexcept;
        const ft_vector<ft_behavior_action> &get_actions() const noexcept;
        void set_actions(const ft_vector<ft_behavior_action> &actions) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
