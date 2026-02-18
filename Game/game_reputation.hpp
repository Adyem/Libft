#ifndef GAME_REPUTATION_HPP
# define GAME_REPUTATION_HPP

#include "../Template/map.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_reputation
{
    private:
        ft_map<int, int>  _milestones;
        ft_map<int, int>  _reps;
        int               _total_rep;
        int               _current_rep;
        mutable pt_mutex *_mutex;
        uint8_t           _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_reputation() noexcept;
        ft_reputation(const ft_map<int, int> &milestones, int total = 0) noexcept;
        virtual ~ft_reputation() noexcept;
        ft_reputation(const ft_reputation &other) = delete;
        ft_reputation &operator=(const ft_reputation &other) = delete;
        ft_reputation(ft_reputation &&other) = delete;
        ft_reputation &operator=(ft_reputation &&other) = delete;

        int initialize() noexcept;
        int initialize(const ft_map<int, int> &milestones, int total = 0) noexcept;
        int initialize(const ft_reputation &other) noexcept;
        int initialize(ft_reputation &&other) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

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

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
