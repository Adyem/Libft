#ifndef GAME_PROGRESS_TRACKER_HPP
# define GAME_PROGRESS_TRACKER_HPP

#include "game_achievement.hpp"
#include "game_quest.hpp"
#include "../Template/map.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_progress_tracker
{
    private:
        ft_map<int, ft_achievement> _achievements;
        ft_map<int, ft_quest>       _quests;
        mutable int                 _error_code;
        mutable pt_mutex            _mutex;
        static thread_local ft_operation_error_stack _operation_errors;

        void set_error(int error) const noexcept;
        static void record_operation_error_unlocked(int error_code);
        static int lock_pair(const ft_progress_tracker &first, const ft_progress_tracker &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_progress_tracker() noexcept;
        ~ft_progress_tracker() noexcept;
        ft_progress_tracker(const ft_progress_tracker &other) noexcept;
        ft_progress_tracker &operator=(const ft_progress_tracker &other) noexcept;
        ft_progress_tracker(ft_progress_tracker &&other) noexcept;
        ft_progress_tracker &operator=(ft_progress_tracker &&other) noexcept;

        ft_map<int, ft_achievement> &get_achievements() noexcept;
        const ft_map<int, ft_achievement> &get_achievements() const noexcept;
        ft_map<int, ft_quest> &get_quests() noexcept;
        const ft_map<int, ft_quest> &get_quests() const noexcept;

        void set_achievements(const ft_map<int, ft_achievement> &achievements) noexcept;
        void set_quests(const ft_map<int, ft_quest> &quests) noexcept;

        int register_achievement(const ft_achievement &achievement) noexcept;
        int register_quest(const ft_quest &quest) noexcept;

        int update_goal_target(int achievement_id, int goal_id, int target) noexcept;
        int update_goal_progress(int achievement_id, int goal_id, int progress) noexcept;
        int add_goal_progress(int achievement_id, int goal_id, int value) noexcept;
        bool is_achievement_complete(int achievement_id) const noexcept;

        int set_quest_phase(int quest_id, int phase) noexcept;
        int advance_quest_phase(int quest_id) noexcept;
        bool is_quest_complete(int quest_id) const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
