#ifndef REPUTATION_HPP
# define REPUTATION_HPP

#include "../Template/map.hpp"
#include "../Errno/errno.hpp"

class ft_reputation
{
    private:
        ft_map<int, int> _milestones;
        ft_map<int, int> _reps;
        int             _total_rep;
        int             _current_rep;
        mutable int     _error;

        void    set_error(int err) const noexcept;

    public:
        ft_reputation() noexcept;
        ft_reputation(const ft_map<int, int> &milestones, int total = 0) noexcept;
        virtual ~ft_reputation() = default;

        int get_total_rep() const noexcept;
        void set_total_rep(int rep) noexcept;
        void add_total_rep(int rep) noexcept;
        void sub_total_rep(int rep) noexcept;

        int get_current_rep() const noexcept;
        void set_current_rep(int rep) noexcept;
        void add_current_rep(int rep) noexcept;
        void sub_current_rep(int rep) noexcept;

        ft_map<int, int>       &get_milestones() noexcept;
        const ft_map<int, int> &get_milestones() const noexcept;
        void set_milestones(const ft_map<int, int> &milestones) noexcept;
        int get_milestone(int id) const noexcept;
        void set_milestone(int id, int value) noexcept;

        ft_map<int, int>       &get_reps() noexcept;
        const ft_map<int, int> &get_reps() const noexcept;
        void set_reps(const ft_map<int, int> &reps) noexcept;
        int get_rep(int id) const noexcept;
        void set_rep(int id, int value) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
