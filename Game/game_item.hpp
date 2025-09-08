#ifndef ITEM_HPP
#define ITEM_HPP

struct ft_item_modifier
{
    int id;
    int value;
};

class ft_item
{
    private:
        int _max_stack;
        int _current_stack;
        int _item_id;
        ft_item_modifier _modifier1;
        ft_item_modifier _modifier2;
        ft_item_modifier _modifier3;
        ft_item_modifier _modifier4;

    public:
        ft_item() noexcept;
        virtual ~ft_item() = default;

        int get_max_stack() const noexcept;
        void set_max_stack(int max) noexcept;

        int get_current_stack() const noexcept;
        void set_current_stack(int amount) noexcept;
        void add_to_stack(int amount) noexcept;
        void sub_from_stack(int amount) noexcept;

        int get_item_id() const noexcept;
        void set_item_id(int id) noexcept;

        ft_item_modifier get_modifier1() const noexcept;
        void set_modifier1(const ft_item_modifier &mod) noexcept;
        int get_modifier1_id() const noexcept;
        void set_modifier1_id(int id) noexcept;
        int get_modifier1_value() const noexcept;
        void set_modifier1_value(int value) noexcept;

        ft_item_modifier get_modifier2() const noexcept;
        void set_modifier2(const ft_item_modifier &mod) noexcept;
        int get_modifier2_id() const noexcept;
        void set_modifier2_id(int id) noexcept;
        int get_modifier2_value() const noexcept;
        void set_modifier2_value(int value) noexcept;

        ft_item_modifier get_modifier3() const noexcept;
        void set_modifier3(const ft_item_modifier &mod) noexcept;
        int get_modifier3_id() const noexcept;
        void set_modifier3_id(int id) noexcept;
        int get_modifier3_value() const noexcept;
        void set_modifier3_value(int value) noexcept;

        ft_item_modifier get_modifier4() const noexcept;
        void set_modifier4(const ft_item_modifier &mod) noexcept;
        int get_modifier4_id() const noexcept;
        void set_modifier4_id(int id) noexcept;
        int get_modifier4_value() const noexcept;
        void set_modifier4_value(int value) noexcept;
};

#endif
