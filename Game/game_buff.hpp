#ifndef GAME_BUFF_HPP
# define GAME_BUFF_HPP

class ft_buff
{
    private:
        int _id;
        int _duration;
        int _modifier1;
        int _modifier2;
        int _modifier3;
        int _modifier4;
        mutable int _error;

        void set_error(int err) const noexcept;

    public:
        ft_buff() noexcept;
        virtual ~ft_buff() = default;
        ft_buff(const ft_buff &other) noexcept;
        ft_buff &operator=(const ft_buff &other) noexcept;
        ft_buff(ft_buff &&other) noexcept;
        ft_buff &operator=(ft_buff &&other) noexcept;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        int get_duration() const noexcept;
        void set_duration(int duration) noexcept;
        void add_duration(int duration) noexcept;
        void sub_duration(int duration) noexcept;

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

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
