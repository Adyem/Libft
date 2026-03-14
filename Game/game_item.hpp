#ifndef GAME_ITEM_HPP
# define GAME_ITEM_HPP

#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/mutex.hpp"

class ft_item_modifier
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t             _id;
        int32_t             _value;
        uint8_t             _initialised_state;
        mutable pt_recursive_mutex *_mutex;
        static thread_local int32_t    _last_error;

        static int32_t set_error(int32_t error_code) noexcept;

    public:
        ft_item_modifier() noexcept;
        virtual ~ft_item_modifier() noexcept;
        ft_item_modifier(const ft_item_modifier &other) noexcept;
        ft_item_modifier &operator=(const ft_item_modifier &other) noexcept = delete;
        ft_item_modifier(ft_item_modifier &&other) noexcept;
        ft_item_modifier &operator=(ft_item_modifier &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t initialize(int32_t id, int32_t value) noexcept;
        int32_t initialize(const ft_item_modifier &other) noexcept;
        int32_t initialize(ft_item_modifier &&other) noexcept;
        int32_t move(ft_item_modifier &other) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;

        int32_t get_id() const noexcept;
        void set_id(int32_t id) noexcept;

        int32_t get_value() const noexcept;
        void set_value(int32_t value) noexcept;

        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

class ft_item
{
    #ifdef LIBFT_TEST_BUILD
        public:
    #else
        private:
    #endif
        int32_t             _max_stack;
        int32_t             _stack_size;
        int32_t             _item_id;
        int32_t             _rarity;
        int32_t             _width;
        int32_t             _height;
        ft_item_modifier _modifier1;
        ft_item_modifier _modifier2;
        ft_item_modifier _modifier3;
        ft_item_modifier _modifier4;
        uint8_t         _initialised_state;
        static thread_local int32_t _last_error;
        mutable pt_recursive_mutex *_mutex;

        static int32_t set_error(int32_t error_code) noexcept;
        int32_t lock_internal(ft_bool *lock_acquired) const noexcept;
        void unlock_internal(ft_bool lock_acquired) const noexcept;
        static int32_t lock_pair(const ft_item &first, const ft_item &second,
                ft_bool *first_locked,
                ft_bool *second_locked);

    public:
        ft_item() noexcept;
        virtual ~ft_item() noexcept;
        ft_item(const ft_item &other) noexcept;
        ft_item &operator=(const ft_item &other) noexcept = delete;
        ft_item(ft_item &&other) noexcept;
        ft_item &operator=(ft_item &&other) noexcept = delete;

        int32_t initialize() noexcept;
        int32_t initialize(const ft_item &other) noexcept;
        int32_t initialize(ft_item &&other) noexcept;
        int32_t move(ft_item &other) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;

        int32_t get_max_stack() const noexcept;
        void set_max_stack(int32_t max) noexcept;

        int32_t get_stack_size() const noexcept;
        void set_stack_size(int32_t amount) noexcept;
        void add_to_stack(int32_t amount) noexcept;
        void sub_from_stack(int32_t amount) noexcept;

        int32_t get_item_id() const noexcept;
        void set_item_id(int32_t id) noexcept;

        int32_t get_width() const noexcept;
        void set_width(int32_t width) noexcept;

        int32_t get_height() const noexcept;
        void set_height(int32_t height) noexcept;

        int32_t get_rarity() const noexcept;
        void set_rarity(int32_t rarity) noexcept;

        int32_t get_modifier1(ft_item_modifier &modifier) const noexcept;
        void set_modifier1(const ft_item_modifier &mod) noexcept;
        int32_t get_modifier1_id() const noexcept;
        void set_modifier1_id(int32_t id) noexcept;
        int32_t get_modifier1_value() const noexcept;
        void set_modifier1_value(int32_t value) noexcept;

        int32_t get_modifier2(ft_item_modifier &modifier) const noexcept;
        void set_modifier2(const ft_item_modifier &mod) noexcept;
        int32_t get_modifier2_id() const noexcept;
        void set_modifier2_id(int32_t id) noexcept;
        int32_t get_modifier2_value() const noexcept;
        void set_modifier2_value(int32_t value) noexcept;

        int32_t get_modifier3(ft_item_modifier &modifier) const noexcept;
        void set_modifier3(const ft_item_modifier &mod) noexcept;
        int32_t get_modifier3_id() const noexcept;
        void set_modifier3_id(int32_t id) noexcept;
        int32_t get_modifier3_value() const noexcept;
        void set_modifier3_value(int32_t value) noexcept;

        int32_t get_modifier4(ft_item_modifier &modifier) const noexcept;
        void set_modifier4(const ft_item_modifier &mod) noexcept;
        int32_t get_modifier4_id() const noexcept;
        void set_modifier4_id(int32_t id) noexcept;
        int32_t get_modifier4_value() const noexcept;
        void set_modifier4_value(int32_t value) noexcept;

        int32_t         get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
