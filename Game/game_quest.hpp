#ifndef GAME_QUEST_HPP
# define GAME_QUEST_HPP

#include "../CPP_class/class_string.hpp"
#include "../Template/vector.hpp"
#include "game_item.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_quest
{
    private:
        int                             _id;
        int                             _phases;
        int                             _current_phase;
        ft_string                       _description;
        ft_string                       _objective;
        int                             _reward_experience;
        ft_vector<ft_sharedptr<ft_item> > _reward_items;
        mutable pt_mutex               *_mutex;
        uint8_t                         _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
                const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_quest() noexcept;
        virtual ~ft_quest() noexcept;
        ft_quest(const ft_quest &other) = delete;
        ft_quest &operator=(const ft_quest &other) = delete;
        ft_quest(ft_quest &&other) = delete;
        ft_quest &operator=(ft_quest &&other) = delete;

        int initialize() noexcept;
        int initialize(const ft_quest &other) noexcept;
        int initialize(ft_quest &&other) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        int get_phases() const noexcept;
        void set_phases(int phases) noexcept;

        int get_current_phase() const noexcept;
        void set_current_phase(int phase) noexcept;

        const ft_string &get_description() const noexcept;
        void set_description(const ft_string &description) noexcept;

        const ft_string &get_objective() const noexcept;
        void set_objective(const ft_string &objective) noexcept;

        int get_reward_experience() const noexcept;
        void set_reward_experience(int experience) noexcept;

        ft_vector<ft_sharedptr<ft_item> >       &get_reward_items() noexcept;
        const ft_vector<ft_sharedptr<ft_item> > &get_reward_items() const noexcept;
        void set_reward_items(const ft_vector<ft_sharedptr<ft_item> > &items) noexcept;

        bool is_complete() const noexcept;
        void advance_phase() noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
