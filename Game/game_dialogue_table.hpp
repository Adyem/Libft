#ifndef GAME_DIALOGUE_TABLE_HPP
# define GAME_DIALOGUE_TABLE_HPP

#include "ft_dialogue_line.hpp"
#include "ft_dialogue_script.hpp"
#include "../Template/map.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_dialogue_table
{
    private:
        ft_map<int, ft_dialogue_line> _lines;
        ft_map<int, ft_dialogue_script> _scripts;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error_code) const noexcept;

    public:
        ft_dialogue_table() noexcept;
        virtual ~ft_dialogue_table() noexcept;
        ft_dialogue_table(const ft_dialogue_table &other) noexcept;
        ft_dialogue_table &operator=(const ft_dialogue_table &other) noexcept;
        ft_dialogue_table(ft_dialogue_table &&other) noexcept;
        ft_dialogue_table &operator=(ft_dialogue_table &&other) noexcept;

        int register_line(const ft_dialogue_line &line) noexcept;
        int register_script(const ft_dialogue_script &script) noexcept;

        int fetch_line(int line_id, ft_dialogue_line &out_line) const noexcept;
        int fetch_script(int script_id, ft_dialogue_script &out_script) const noexcept;

        ft_map<int, ft_dialogue_line> &get_lines() noexcept;
        const ft_map<int, ft_dialogue_line> &get_lines() const noexcept;
        void set_lines(const ft_map<int, ft_dialogue_line> &lines) noexcept;

        ft_map<int, ft_dialogue_script> &get_scripts() noexcept;
        const ft_map<int, ft_dialogue_script> &get_scripts() const noexcept;
        void set_scripts(const ft_map<int, ft_dialogue_script> &scripts) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
