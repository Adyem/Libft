#ifndef ACHIEVEMENT_HPP
# define ACHIEVEMENT_HPP

#include "../Template/map.hpp"
#include "../Errno/errno.hpp"

struct ft_goal
{
    int goal;
    int progress;
};

class ft_achievement
{
    private:
        int _id;
        ft_map<int, ft_goal> _goals;
        mutable int         _error;

        void set_error(int err) const noexcept;

    public:
        ft_achievement() noexcept;
        virtual ~ft_achievement() = default;

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
