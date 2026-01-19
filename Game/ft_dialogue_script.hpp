#ifndef FT_DIALOGUE_SCRIPT_HPP
# define FT_DIALOGUE_SCRIPT_HPP

#include "ft_dialogue_line.hpp"
#include "../Template/vector.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_dialogue_script
{
    private:
        int _script_id;
        ft_string _title;
        ft_string _summary;
        int _start_line_id;
        ft_vector<ft_dialogue_line> _lines;
        mutable int _error_code;
        mutable pt_mutex _mutex;
        static thread_local ft_operation_error_stack _operation_errors;

        void set_error(int error_code) const noexcept;
        static void record_operation_error_unlocked(int error_code);
        static int lock_pair(const ft_dialogue_script &first, const ft_dialogue_script &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_dialogue_script() noexcept;
        ft_dialogue_script(int script_id, const ft_string &title, const ft_string &summary,
                int start_line_id, const ft_vector<ft_dialogue_line> &lines) noexcept;
        virtual ~ft_dialogue_script() noexcept;
        ft_dialogue_script(const ft_dialogue_script &other) noexcept;
        ft_dialogue_script &operator=(const ft_dialogue_script &other) noexcept;
        ft_dialogue_script(ft_dialogue_script &&other) noexcept;
        ft_dialogue_script &operator=(ft_dialogue_script &&other) noexcept;

        int get_script_id() const noexcept;
        void set_script_id(int script_id) noexcept;

        const ft_string &get_title() const noexcept;
        void set_title(const ft_string &title) noexcept;

        const ft_string &get_summary() const noexcept;
        void set_summary(const ft_string &summary) noexcept;

        int get_start_line_id() const noexcept;
        void set_start_line_id(int start_line_id) noexcept;

        const ft_vector<ft_dialogue_line> &get_lines() const noexcept;
        ft_vector<ft_dialogue_line> &get_lines() noexcept;
        void set_lines(const ft_vector<ft_dialogue_line> &lines) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
