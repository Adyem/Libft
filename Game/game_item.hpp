#ifndef GAME_ITEM_HPP
# define GAME_ITEM_HPP

#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_item_modifier
{
    private:
        int             _id;
        int             _value;
        mutable int     _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error_code) const noexcept;
        static int lock_pair(const ft_item_modifier &first,
                const ft_item_modifier &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_item_modifier() noexcept;
        ft_item_modifier(int id, int value) noexcept;
        virtual ~ft_item_modifier() = default;
        ft_item_modifier(const ft_item_modifier &other) noexcept;
        ft_item_modifier &operator=(const ft_item_modifier &other) noexcept;
        ft_item_modifier(ft_item_modifier &&other) noexcept;
        ft_item_modifier &operator=(ft_item_modifier &&other) noexcept;

        int get_id() const noexcept;
        void set_id(int id) noexcept;

        int get_value() const noexcept;
        void set_value(int value) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

class ft_item
{
    private:
        int             _max_stack;
        int             _stack_size;
        int             _item_id;
        int             _rarity;
        int             _width;
        int             _height;
        ft_item_modifier _modifier1;
        ft_item_modifier _modifier2;
        ft_item_modifier _modifier3;
        ft_item_modifier _modifier4;
        mutable int     _error_code;
        mutable pt_mutex _mutex;

        void set_error(int err) const noexcept;
        static int lock_pair(const ft_item &first, const ft_item &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);

    public:
        ft_item() noexcept;
        virtual ~ft_item() = default;
        ft_item(const ft_item &other) noexcept;
        ft_item &operator=(const ft_item &other) noexcept;
        ft_item(ft_item &&other) noexcept;
        ft_item &operator=(ft_item &&other) noexcept;

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

        int         get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
