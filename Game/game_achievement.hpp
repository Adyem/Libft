#ifndef GAME_ACHIEVEMENT_HPP
# define GAME_ACHIEVEMENT_HPP

#include "../Template/map.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_goal
{
    private:
        int              _target;
        int              _progress;
        mutable int      _error;
        mutable pt_mutex _mutex;

        void set_error(int error) const noexcept;
        static int lock_pair(const ft_goal &first,
                const ft_goal &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_goal() noexcept;
        ~ft_goal() noexcept;
        ft_goal(const ft_goal &other) noexcept;
        ft_goal &operator=(const ft_goal &other) noexcept;
        ft_goal(ft_goal &&other) noexcept;
        ft_goal &operator=(ft_goal &&other) noexcept;

        int  get_target() const noexcept;
        void set_target(int target) noexcept;
        int  get_progress() const noexcept;
        void set_progress(int value) noexcept;
        void add_progress(int delta) noexcept;

        int         get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

class ft_achievement
{
    private:
        int _id;
        ft_map<int, ft_goal> _goals;
        mutable int         _error;
        mutable pt_mutex    _mutex;

        void set_error(int err) const noexcept;
        static int lock_pair(const ft_achievement &first,
                const ft_achievement &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_achievement() noexcept;
        virtual ~ft_achievement() noexcept;
        ft_achievement(const ft_achievement &other) noexcept;
        ft_achievement &operator=(const ft_achievement &other) noexcept;
        ft_achievement(ft_achievement &&other) noexcept;
        ft_achievement &operator=(ft_achievement &&other) noexcept;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        ft_map<int, ft_goal>       &get_goals() noexcept;
        const ft_map<int, ft_goal> &get_goals() const noexcept;
        void set_goals(const ft_map<int, ft_goal> &goals) noexcept;
        int  get_goal(int id) const noexcept;
        void set_goal(int id, int goal) noexcept;

        int  get_progress(int id) const noexcept;
        void set_progress(int id, int progress) noexcept;
        void add_progress(int id, int value) noexcept;

        bool is_goal_complete(int id) const noexcept;
        bool is_complete() const noexcept;

        int         get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
