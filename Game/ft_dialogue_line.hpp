#ifndef FT_DIALOGUE_LINE_HPP
# define FT_DIALOGUE_LINE_HPP

#include "../CPP_class/class_string.hpp"
#include "../Template/vector.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"

class ft_dialogue_line
{
    private:
        int _line_id;
        ft_string _speaker;
        ft_string _text;
        ft_vector<int> _next_line_ids;
        mutable pt_mutex _mutex;

    public:
        ft_dialogue_line() noexcept;
        ft_dialogue_line(int line_id, const ft_string &speaker, const ft_string &text,
                const ft_vector<int> &next_line_ids) noexcept;
        virtual ~ft_dialogue_line() noexcept;
        ft_dialogue_line(const ft_dialogue_line &other) noexcept = delete;
        ft_dialogue_line &operator=(const ft_dialogue_line &other) noexcept = delete;
        ft_dialogue_line(ft_dialogue_line &&other) noexcept = delete;
        ft_dialogue_line &operator=(ft_dialogue_line &&other) noexcept = delete;

        int get_line_id() const noexcept;
        void set_line_id(int line_id) noexcept;

        const ft_string &get_speaker() const noexcept;
        void set_speaker(const ft_string &speaker) noexcept;

        const ft_string &get_text() const noexcept;
        void set_text(const ft_string &text) noexcept;

        const ft_vector<int> &get_next_line_ids() const noexcept;
        ft_vector<int> &get_next_line_ids() noexcept;
        void set_next_line_ids(const ft_vector<int> &next_line_ids) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
