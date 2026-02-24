#ifndef FT_DIALOGUE_SCRIPT_HPP
# define FT_DIALOGUE_SCRIPT_HPP

#include "../CPP_class/class_string.hpp"
#include "../Template/shared_ptr.hpp"
#include "ft_dialogue_line.hpp"
#include "../Template/vector.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"
#include <stdint.h>

class ft_dialogue_script
{
    private:
        int _script_id;
        ft_string _title;
        ft_string _summary;
        int _start_line_id;
        ft_vector<ft_sharedptr<ft_dialogue_line> > _lines;
        pt_recursive_mutex *_mutex;
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
        ft_dialogue_script() noexcept;
        virtual ~ft_dialogue_script() noexcept;
        ft_dialogue_script(const ft_dialogue_script &other) noexcept = delete;
        ft_dialogue_script &operator=(const ft_dialogue_script &other) noexcept = delete;
        ft_dialogue_script(ft_dialogue_script &&other) noexcept = delete;
        ft_dialogue_script &operator=(ft_dialogue_script &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const ft_dialogue_script &other) noexcept;
        int initialize(ft_dialogue_script &&other) noexcept;
        int initialize(int script_id, const ft_string &title,
            const ft_string &summary, int start_line_id,
            const ft_vector<ft_dialogue_line> &lines) noexcept;
        int destroy() noexcept;

        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int get_script_id() const noexcept;
        void set_script_id(int script_id) noexcept;

        const ft_string &get_title() const noexcept;
        void set_title(const ft_string &title) noexcept;

        const ft_string &get_summary() const noexcept;
        void set_summary(const ft_string &summary) noexcept;

        int get_start_line_id() const noexcept;
        void set_start_line_id(int start_line_id) noexcept;

        const ft_vector<ft_sharedptr<ft_dialogue_line> > &get_lines() const noexcept;
        ft_vector<ft_sharedptr<ft_dialogue_line> > &get_lines() noexcept;
        void set_lines(const ft_vector<ft_sharedptr<ft_dialogue_line> > &lines) noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
