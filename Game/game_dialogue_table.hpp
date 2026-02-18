#ifndef GAME_DIALOGUE_TABLE_HPP
# define GAME_DIALOGUE_TABLE_HPP

#include "ft_dialogue_line.hpp"
#include "ft_dialogue_script.hpp"
#include "../Template/map.hpp"
#include "../PThread/mutex.hpp"
#include "../Template/shared_ptr.hpp"
#include <stdint.h>

class ft_dialogue_table
{
    private:
        ft_map<int, ft_sharedptr<ft_dialogue_line> > _lines;
        ft_map<int, ft_dialogue_script> _scripts;
        pt_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_dialogue_table() noexcept;
        virtual ~ft_dialogue_table() noexcept;
        ft_dialogue_table(const ft_dialogue_table &other) noexcept = delete;
        ft_dialogue_table &operator=(const ft_dialogue_table &other) noexcept = delete;
        ft_dialogue_table(ft_dialogue_table &&other) noexcept = delete;
        ft_dialogue_table &operator=(ft_dialogue_table &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const ft_dialogue_table &other) noexcept;
        int initialize(ft_dialogue_table &&other) noexcept;
        int destroy() noexcept;

        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int register_line(const ft_dialogue_line &line) noexcept;
        int register_script(const ft_dialogue_script &script) noexcept;

        int fetch_line(int line_id, ft_dialogue_line &out_line) const noexcept;
        int fetch_script(int script_id, ft_dialogue_script &out_script) const noexcept;

        ft_map<int, ft_sharedptr<ft_dialogue_line> > &get_lines() noexcept;
        const ft_map<int, ft_sharedptr<ft_dialogue_line> > &get_lines() const noexcept;
        void set_lines(const ft_map<int, ft_sharedptr<ft_dialogue_line> > &lines) noexcept;

        ft_map<int, ft_dialogue_script> &get_scripts() noexcept;
        const ft_map<int, ft_dialogue_script> &get_scripts() const noexcept;
        void set_scripts(const ft_map<int, ft_dialogue_script> &scripts) noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
