#ifndef GAME_REPUTATION_HPP
# define GAME_REPUTATION_HPP

#include "../Template/map.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_reputation
{
    private:
        ft_map<int, int> _milestones;
        ft_map<int, int> _reps;
        int             _total_rep;
        int             _current_rep;
        mutable int     _error;
        mutable pt_mutex _mutex;

        void    set_error(int err) const noexcept;
        static int lock_pair(const ft_reputation &first, const ft_reputation &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_reputation() noexcept;
        ft_reputation(const ft_map<int, int> &milestones, int total = 0) noexcept;
        virtual ~ft_reputation() = default;
        ft_reputation(const ft_reputation &other) noexcept;
        ft_reputation &operator=(const ft_reputation &other) noexcept;
        ft_reputation(ft_reputation &&other) noexcept;
        ft_reputation &operator=(ft_reputation &&other) noexcept;

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
