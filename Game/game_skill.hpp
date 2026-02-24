#ifndef GAME_SKILL_HPP
# define GAME_SKILL_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_skill
{
    private:
        int              _id;
        int              _level;
        int              _cooldown;
        int              _modifier1;
        int              _modifier2;
        int              _modifier3;
        int              _modifier4;
        mutable pt_recursive_mutex *_mutex;
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
        ft_skill() noexcept;
        virtual ~ft_skill() noexcept;
        ft_skill(const ft_skill &other) = delete;
        ft_skill &operator=(const ft_skill &other) = delete;
        ft_skill(ft_skill &&other) = delete;
        ft_skill &operator=(ft_skill &&other) = delete;

        int initialize() noexcept;
        int initialize(const ft_skill &other) noexcept;
        int initialize(ft_skill &&other) noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        int get_level() const noexcept;
        void set_level(int level) noexcept;

        int get_cooldown() const noexcept;
        void set_cooldown(int cooldown) noexcept;
        void add_cooldown(int cooldown) noexcept;
        void sub_cooldown(int cooldown) noexcept;

        int get_modifier1() const noexcept;
        void set_modifier1(int mod) noexcept;
        void add_modifier1(int mod) noexcept;
        void sub_modifier1(int mod) noexcept;

        int get_modifier2() const noexcept;
        void set_modifier2(int mod) noexcept;
        void add_modifier2(int mod) noexcept;
        void sub_modifier2(int mod) noexcept;

        int get_modifier3() const noexcept;
        void set_modifier3(int mod) noexcept;
        void add_modifier3(int mod) noexcept;
        void sub_modifier3(int mod) noexcept;

        int get_modifier4() const noexcept;
        void set_modifier4(int mod) noexcept;
        void add_modifier4(int mod) noexcept;
        void sub_modifier4(int mod) noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
