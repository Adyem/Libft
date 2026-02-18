#ifndef GAME_ACHIEVEMENT_HPP
# define GAME_ACHIEVEMENT_HPP

#include <cstdint>
#include "../Template/map.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"

class ft_goal
{
    private:
        int              _target;
        int              _progress;
        mutable pt_mutex *_mutex;
        uint8_t          _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_goal() noexcept;
        ~ft_goal() noexcept;
        ft_goal(const ft_goal &other) = delete;
        ft_goal &operator=(const ft_goal &other) = delete;
        ft_goal(ft_goal &&other) = delete;
        ft_goal &operator=(ft_goal &&other) = delete;

        int initialize() noexcept;
        int initialize(const ft_goal &other) noexcept;
        int initialize(ft_goal &&other) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int get_target() const noexcept;
        void set_target(int target) noexcept;
        int get_progress() const noexcept;
        void set_progress(int value) noexcept;
        void add_progress(int delta) noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

class ft_achievement
{
    private:
        int              _id;
        ft_map<int, ft_goal> _goals;
        mutable pt_mutex *_mutex;
        uint8_t          _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_achievement() noexcept;
        virtual ~ft_achievement() noexcept;
        ft_achievement(const ft_achievement &other) = delete;
        ft_achievement &operator=(const ft_achievement &other) = delete;
        ft_achievement(ft_achievement &&other) = delete;
        ft_achievement &operator=(ft_achievement &&other) = delete;

        int initialize() noexcept;
        int initialize(const ft_achievement &other) noexcept;
        int initialize(ft_achievement &&other) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        ft_map<int, ft_goal>       &get_goals() noexcept;
        const ft_map<int, ft_goal> &get_goals() const noexcept;
        void set_goals(const ft_map<int, ft_goal> &goals) noexcept;
        int get_goal(int id) const noexcept;
        void set_goal(int id, int goal) noexcept;

        int get_progress(int id) const noexcept;
        void set_progress(int id, int progress) noexcept;
        void add_progress(int id, int value) noexcept;

        bool is_goal_complete(int id) const noexcept;
        bool is_complete() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
