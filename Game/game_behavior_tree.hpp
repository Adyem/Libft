#ifndef GAME_BEHAVIOR_TREE_HPP
# define GAME_BEHAVIOR_TREE_HPP

#include "../Template/vector.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Template/function.hpp"
#include "../Errno/errno.hpp"

class ft_character;

#define FT_BEHAVIOR_STATUS_RUNNING 0
#define FT_BEHAVIOR_STATUS_SUCCESS 1
#define FT_BEHAVIOR_STATUS_FAILURE 2

class ft_behavior_context
{
    private:
        ft_character        *_character;
        void                *_user_data;

    public:
        ft_behavior_context() noexcept;
        ~ft_behavior_context() noexcept;
        ft_behavior_context(const ft_behavior_context &other) noexcept;
        ft_behavior_context &operator=(const ft_behavior_context &other) noexcept;

        ft_character *get_character() const noexcept;
        void set_character(ft_character *character) noexcept;

        void *get_user_data() const noexcept;
        void set_user_data(void *user_data) noexcept;
};

class ft_behavior_node
{
    protected:
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

    public:
        ft_behavior_node() noexcept;
        virtual ~ft_behavior_node() noexcept;
        ft_behavior_node(const ft_behavior_node &other) noexcept;
        ft_behavior_node &operator=(const ft_behavior_node &other) noexcept;

        virtual int tick(ft_behavior_context &context) noexcept = 0;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

class ft_behavior_action : public ft_behavior_node
{
    private:
        ft_function<int(ft_behavior_context &)> _callback;

    public:
        ft_behavior_action() noexcept;
        explicit ft_behavior_action(const ft_function<int(ft_behavior_context &)> &callback) noexcept;
        virtual ~ft_behavior_action() noexcept;
        ft_behavior_action(const ft_behavior_action &other) noexcept;
        ft_behavior_action &operator=(const ft_behavior_action &other) noexcept;

        void set_callback(const ft_function<int(ft_behavior_context &)> &callback) noexcept;
        const ft_function<int(ft_behavior_context &)> &get_callback() const noexcept;

        virtual int tick(ft_behavior_context &context) noexcept;
};

class ft_behavior_composite : public ft_behavior_node
{
    protected:
        ft_vector<ft_sharedptr<ft_behavior_node> > _children;

        bool validate_child(const ft_sharedptr<ft_behavior_node> &child) const noexcept;

    public:
        ft_behavior_composite() noexcept;
        virtual ~ft_behavior_composite() noexcept;
        ft_behavior_composite(const ft_behavior_composite &other) noexcept;
        ft_behavior_composite &operator=(const ft_behavior_composite &other) noexcept;

        void add_child(const ft_sharedptr<ft_behavior_node> &child) noexcept;
        void clear_children() noexcept;
        ft_vector<ft_sharedptr<ft_behavior_node> > &get_children() noexcept;
        const ft_vector<ft_sharedptr<ft_behavior_node> > &get_children() const noexcept;
};

class ft_behavior_selector : public ft_behavior_composite
{
    public:
        ft_behavior_selector() noexcept;
        virtual ~ft_behavior_selector() noexcept;
        ft_behavior_selector(const ft_behavior_selector &other) noexcept;
        ft_behavior_selector &operator=(const ft_behavior_selector &other) noexcept;

        virtual int tick(ft_behavior_context &context) noexcept;
};

class ft_behavior_sequence : public ft_behavior_composite
{
    public:
        ft_behavior_sequence() noexcept;
        virtual ~ft_behavior_sequence() noexcept;
        ft_behavior_sequence(const ft_behavior_sequence &other) noexcept;
        ft_behavior_sequence &operator=(const ft_behavior_sequence &other) noexcept;

        virtual int tick(ft_behavior_context &context) noexcept;
};

class ft_behavior_tree
{
    private:
        ft_sharedptr<ft_behavior_node> _root;
        mutable int                     _error_code;

        void set_error(int error_code) const noexcept;

    public:
        ft_behavior_tree() noexcept;
        ~ft_behavior_tree() noexcept;
        ft_behavior_tree(const ft_behavior_tree &other) noexcept;
        ft_behavior_tree &operator=(const ft_behavior_tree &other) noexcept;

        void set_root(const ft_sharedptr<ft_behavior_node> &root) noexcept;
        ft_sharedptr<ft_behavior_node> &get_root() noexcept;
        const ft_sharedptr<ft_behavior_node> &get_root() const noexcept;

        int tick(ft_behavior_context &context) noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
