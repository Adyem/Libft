#ifndef GAME_SKILL_HPP
# define GAME_SKILL_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_skill
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t              _id;
        int32_t              _level;
        int32_t              _cooldown;
        int32_t              _modifier1;
        int32_t              _modifier2;
        int32_t              _modifier3;
        int32_t              _modifier4;
        mutable pt_recursive_mutex *_mutex;
        uint8_t          _initialised_state;
        static thread_local int32_t    _last_error;
        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        int32_t unlock_internal(ft_bool lock_acquired) const noexcept;
        static int32_t set_error(int32_t error_code) noexcept;
    public:
        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
        ft_skill() noexcept;
        ft_skill(const ft_skill &other) noexcept;
        ft_skill(ft_skill &&other) noexcept;
        virtual ~ft_skill() noexcept;
        ft_skill &operator=(const ft_skill &other) = delete;
        ft_skill &operator=(ft_skill &&other) = delete;

        int32_t initialize() noexcept;
        int32_t initialize(const ft_skill &other) noexcept;
        int32_t initialize(ft_skill &&other) noexcept;
        int32_t move(ft_skill &other) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t lock(ft_bool *lock_acquired) const noexcept;
        void unlock(ft_bool lock_acquired) const noexcept;

        int32_t get_id() const noexcept;
        void set_id(int32_t id) noexcept;

        int32_t get_level() const noexcept;
        void set_level(int32_t level) noexcept;

        int32_t get_cooldown() const noexcept;
        void set_cooldown(int32_t cooldown) noexcept;
        void add_cooldown(int32_t cooldown) noexcept;
        void sub_cooldown(int32_t cooldown) noexcept;

        int32_t get_modifier1() const noexcept;
        void set_modifier1(int32_t mod) noexcept;
        void add_modifier1(int32_t mod) noexcept;
        void sub_modifier1(int32_t mod) noexcept;

        int32_t get_modifier2() const noexcept;
        void set_modifier2(int32_t mod) noexcept;
        void add_modifier2(int32_t mod) noexcept;
        void sub_modifier2(int32_t mod) noexcept;

        int32_t get_modifier3() const noexcept;
        void set_modifier3(int32_t mod) noexcept;
        void add_modifier3(int32_t mod) noexcept;
        void sub_modifier3(int32_t mod) noexcept;

        int32_t get_modifier4() const noexcept;
        void set_modifier4(int32_t mod) noexcept;
        void add_modifier4(int32_t mod) noexcept;
        void sub_modifier4(int32_t mod) noexcept;

};

#endif
