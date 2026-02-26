#ifndef GAME_ITEM_HPP
# define GAME_ITEM_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"

class ft_item_modifier
{
    private:
        int             _id;
        int             _value;
        mutable pt_recursive_mutex *_mutex;
        static thread_local int    _last_error;

        void set_error(int error_code) const noexcept;

    public:
        ft_item_modifier() noexcept;
        virtual ~ft_item_modifier() noexcept;
        ft_item_modifier(const ft_item_modifier &other) noexcept = delete;
        ft_item_modifier &operator=(const ft_item_modifier &other) noexcept = delete;
        ft_item_modifier(ft_item_modifier &&other) noexcept = delete;
        ft_item_modifier &operator=(ft_item_modifier &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(int id, int value) noexcept;
        int initialize(const ft_item_modifier &other) noexcept;
        int initialize(ft_item_modifier &&other) noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;

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
        static thread_local int _last_error;
        mutable pt_recursive_mutex *_mutex;

        void set_error(int err) const noexcept;
        int lock_internal(bool *lock_acquired) const noexcept;
        void unlock_internal(bool lock_acquired) const noexcept;
        static int lock_pair(const ft_item &first, const ft_item &second,
                bool *first_locked,
                bool *second_locked);

    public:
        ft_item() noexcept;
        virtual ~ft_item() noexcept;
        ft_item(const ft_item &other) noexcept = delete;
        ft_item &operator=(const ft_item &other) noexcept = delete;
        ft_item(ft_item &&other) noexcept = delete;
        ft_item &operator=(ft_item &&other) noexcept = delete;

        int initialize() noexcept;
        int initialize(const ft_item &other) noexcept;
        int initialize(ft_item &&other) noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;

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

        int get_modifier1(ft_item_modifier &modifier) const noexcept;
        void set_modifier1(const ft_item_modifier &mod) noexcept;
        int get_modifier1_id() const noexcept;
        void set_modifier1_id(int id) noexcept;
        int get_modifier1_value() const noexcept;
        void set_modifier1_value(int value) noexcept;

        int get_modifier2(ft_item_modifier &modifier) const noexcept;
        void set_modifier2(const ft_item_modifier &mod) noexcept;
        int get_modifier2_id() const noexcept;
        void set_modifier2_id(int id) noexcept;
        int get_modifier2_value() const noexcept;
        void set_modifier2_value(int value) noexcept;

        int get_modifier3(ft_item_modifier &modifier) const noexcept;
        void set_modifier3(const ft_item_modifier &mod) noexcept;
        int get_modifier3_id() const noexcept;
        void set_modifier3_id(int id) noexcept;
        int get_modifier3_value() const noexcept;
        void set_modifier3_value(int value) noexcept;

        int get_modifier4(ft_item_modifier &modifier) const noexcept;
        void set_modifier4(const ft_item_modifier &mod) noexcept;
        int get_modifier4_id() const noexcept;
        void set_modifier4_id(int id) noexcept;
        int get_modifier4_value() const noexcept;
        void set_modifier4_value(int value) noexcept;

        int         get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
