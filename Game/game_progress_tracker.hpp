#ifndef GAME_PROGRESS_TRACKER_HPP
# define GAME_PROGRESS_TRACKER_HPP

#include "game_achievement.hpp"
#include "game_quest.hpp"
#include "../Template/map.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_progress_tracker
{
    private:
        ft_map<int, ft_achievement> _achievements;
        ft_map<int, ft_quest>       _quests;
        pt_mutex                   *_mutex;
        uint8_t                     _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_progress_tracker() noexcept;
        ~ft_progress_tracker() noexcept;
        ft_progress_tracker(const ft_progress_tracker &other) noexcept = delete;
        ft_progress_tracker &operator=(const ft_progress_tracker &other) noexcept = delete;
        ft_progress_tracker(ft_progress_tracker &&other) noexcept = delete;
        ft_progress_tracker &operator=(ft_progress_tracker &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const ft_progress_tracker &other) noexcept;
        int initialize(ft_progress_tracker &&other) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

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

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
