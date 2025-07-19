#ifndef DEBUFF_HPP
# define DEBUFF_HPP

class ft_debuff
{
    private:
        int _id;
        int _duration;
        int _modifier1;
        int _modifier2;
        int _modifier3;
        int _modifier4;

    public:
        ft_debuff() noexcept;
        virtual ~ft_debuff() = default;

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
};

#endif
