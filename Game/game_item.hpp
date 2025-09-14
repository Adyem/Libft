#ifndef GAME_ITEM_HPP
# define GAME_ITEM_HPP

struct ft_item_modifier
{
    int id;
    int value;
};

class ft_item
{
    private:
        int _max_stack;
        int _stack_size;
        int _item_id;
        int _rarity;
        int _width;
        int _height;
        ft_item_modifier _modifier1;
        ft_item_modifier _modifier2;
        ft_item_modifier _modifier3;
        ft_item_modifier _modifier4;

    public:
        ft_item() noexcept;
        virtual ~ft_item() = default;

        int get_max_stack() const noexcept;
        void set_max_stack(int max) noexcept;

        int get_stack_size() const noexcept;
        void set_stack_size(int amount) noexcept;
        void add_to_stack(int amount) noexcept;
        void sub_from_stack(int amount) noexcept;

        int get_item_id() const noexcept;
        void set_item_id(int id) noexcept;

        int get_width() const noexcept;
        void set_width(int width) noexcept;

        int get_height() const noexcept;
        void set_height(int height) noexcept;

        int get_rarity() const noexcept;
        void set_rarity(int rarity) noexcept;

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
