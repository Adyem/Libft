#include "game_behavior_tree.hpp"
#include "../Template/move.hpp"
#include "../Basic/basic.hpp"

ft_behavior_context::ft_behavior_context() noexcept
    : _character(ft_nullptr), _user_data(ft_nullptr)
{
    return ;
}

ft_behavior_context::~ft_behavior_context() noexcept
{
    return ;
}

ft_behavior_context::ft_behavior_context(const ft_behavior_context &other) noexcept
    : _character(other._character), _user_data(other._user_data)
{
    return ;
}

ft_behavior_context &ft_behavior_context::operator=(const ft_behavior_context &other) noexcept
{
    if (this != &other)
    {
        this->_character = other._character;
        this->_user_data = other._user_data;
    }
    return (*this);
}

ft_character *ft_behavior_context::get_character() const noexcept
{
    return (this->_character);
}

void ft_behavior_context::set_character(ft_character *character) noexcept
{
    this->_character = character;
    return ;
}

void *ft_behavior_context::get_user_data() const noexcept
{
    return (this->_user_data);
}

void ft_behavior_context::set_user_data(void *user_data) noexcept
{
    this->_user_data = user_data;
    return ;
}

ft_behavior_node::ft_behavior_node() noexcept
    : _error_code(FT_ERR_SUCCESS)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_behavior_node::~ft_behavior_node() noexcept
{
    return ;
}

ft_behavior_node::ft_behavior_node(const ft_behavior_node &other) noexcept
    : _error_code(other._error_code)
{
    this->set_error(other._error_code);
    return ;
}

ft_behavior_node &ft_behavior_node::operator=(const ft_behavior_node &other) noexcept
{
    if (this != &other)
        this->set_error(other._error_code);
    return (*this);
}

void ft_behavior_node::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    return ;
}

int ft_behavior_node::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_behavior_node::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

ft_behavior_action::ft_behavior_action() noexcept
    : ft_behavior_node(), _callback()
{
    return ;
}

ft_behavior_action::ft_behavior_action(const ft_function<int(ft_behavior_context &)> &callback) noexcept
    : ft_behavior_node(), _callback(callback)
{
    return ;
}

ft_behavior_action::~ft_behavior_action() noexcept
{
    return ;
}

ft_behavior_action::ft_behavior_action(const ft_behavior_action &other) noexcept
    : ft_behavior_node(other), _callback(other._callback)
{
    return ;
}

ft_behavior_action &ft_behavior_action::operator=(const ft_behavior_action &other) noexcept
{
    if (this != &other)
    {
        ft_behavior_node::operator=(other);
        this->_callback = other._callback;
    }
    return (*this);
}

void ft_behavior_action::set_callback(const ft_function<int(ft_behavior_context &)> &callback) noexcept
{
    this->_callback = callback;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

const ft_function<int(ft_behavior_context &)> &ft_behavior_action::get_callback() const noexcept
{
    return (this->_callback);
}

int ft_behavior_action::tick(ft_behavior_context &context) noexcept
{
    if (!this->_callback)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_BEHAVIOR_STATUS_FAILURE);
    }
    int status;

    status = this->_callback(context);
    if (status != FT_BEHAVIOR_STATUS_RUNNING
        && status != FT_BEHAVIOR_STATUS_SUCCESS
        && status != FT_BEHAVIOR_STATUS_FAILURE)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_BEHAVIOR_STATUS_FAILURE);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (status);
}

ft_behavior_composite::ft_behavior_composite() noexcept
    : ft_behavior_node(), _children()
{
    return ;
}

ft_behavior_composite::~ft_behavior_composite() noexcept
{
    return ;
}

ft_behavior_composite::ft_behavior_composite(const ft_behavior_composite &other) noexcept
    : ft_behavior_node(other), _children()
{
    size_t index;
    size_t count;

    index = 0;
    count = other._children.size();
    while (index < count)
    {
        this->_children.push_back(other._children[index]);
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return ;
        }
        index++;
    }
    return ;
}

ft_behavior_composite &ft_behavior_composite::operator=(const ft_behavior_composite &other) noexcept
{
    if (this != &other)
    {
        ft_behavior_node::operator=(other);
        this->_children.clear();
        if (false)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (*this);
        }
        size_t index;
        size_t count;

        index = 0;
        count = other._children.size();
        while (index < count)
        {
            this->_children.push_back(other._children[index]);
            if (false)
            {
                this->set_error(FT_ERR_SUCCESS);
                return (*this);
            }
            index++;
        }
    }
    return (*this);
}

bool ft_behavior_composite::validate_child(const ft_sharedptr<ft_behavior_node> &child) const noexcept
{
    if (!child)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (child->get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(child->get_error());
        return (false);
    }
    return (true);
}

void ft_behavior_composite::add_child(const ft_sharedptr<ft_behavior_node> &child) noexcept
{
    if (!child)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_children.push_back(child);
    if (false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_behavior_composite::clear_children() noexcept
{
    this->_children.clear();
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_vector<ft_sharedptr<ft_behavior_node> > &ft_behavior_composite::get_children() noexcept
{
    this->set_error(FT_ERR_SUCCESS);
    return (this->_children);
}

const ft_vector<ft_sharedptr<ft_behavior_node> > &ft_behavior_composite::get_children() const noexcept
{
    const_cast<ft_behavior_composite *>(this)->set_error(FT_ERR_SUCCESS);
    return (this->_children);
}

ft_behavior_selector::ft_behavior_selector() noexcept
    : ft_behavior_composite()
{
    return ;
}

ft_behavior_selector::~ft_behavior_selector() noexcept
{
    return ;
}

ft_behavior_selector::ft_behavior_selector(const ft_behavior_selector &other) noexcept
    : ft_behavior_composite(other)
{
    return ;
}

ft_behavior_selector &ft_behavior_selector::operator=(const ft_behavior_selector &other) noexcept
{
    if (this != &other)
        ft_behavior_composite::operator=(other);
    return (*this);
}

int ft_behavior_selector::tick(ft_behavior_context &context) noexcept
{
    size_t index;
    size_t child_count;

    index = 0;
    child_count = this->_children.size();
    while (index < child_count)
    {
        ft_sharedptr<ft_behavior_node> child = this->_children[index];
        if (this->validate_child(child) == false)
            return (FT_BEHAVIOR_STATUS_FAILURE);
        int status = child->tick(context);
        if (child->get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(child->get_error());
            return (FT_BEHAVIOR_STATUS_FAILURE);
        }
        if (status == FT_BEHAVIOR_STATUS_SUCCESS)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_BEHAVIOR_STATUS_SUCCESS);
        }
        if (status == FT_BEHAVIOR_STATUS_RUNNING)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_BEHAVIOR_STATUS_RUNNING);
        }
        index++;
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_BEHAVIOR_STATUS_FAILURE);
}

ft_behavior_sequence::ft_behavior_sequence() noexcept
    : ft_behavior_composite()
{
    return ;
}

ft_behavior_sequence::~ft_behavior_sequence() noexcept
{
    return ;
}

ft_behavior_sequence::ft_behavior_sequence(const ft_behavior_sequence &other) noexcept
    : ft_behavior_composite(other)
{
    return ;
}

ft_behavior_sequence &ft_behavior_sequence::operator=(const ft_behavior_sequence &other) noexcept
{
    if (this != &other)
        ft_behavior_composite::operator=(other);
    return (*this);
}

int ft_behavior_sequence::tick(ft_behavior_context &context) noexcept
{
    size_t index;
    size_t child_count;

    index = 0;
    child_count = this->_children.size();
    if (child_count == 0)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_BEHAVIOR_STATUS_SUCCESS);
    }
    while (index < child_count)
    {
        ft_sharedptr<ft_behavior_node> child = this->_children[index];
        if (this->validate_child(child) == false)
            return (FT_BEHAVIOR_STATUS_FAILURE);
        int status = child->tick(context);
        if (child->get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(child->get_error());
            return (FT_BEHAVIOR_STATUS_FAILURE);
        }
        if (status == FT_BEHAVIOR_STATUS_FAILURE)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_BEHAVIOR_STATUS_FAILURE);
        }
        if (status == FT_BEHAVIOR_STATUS_RUNNING)
        {
            this->set_error(FT_ERR_SUCCESS);
            return (FT_BEHAVIOR_STATUS_RUNNING);
        }
        index++;
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_BEHAVIOR_STATUS_SUCCESS);
}

ft_behavior_tree::ft_behavior_tree() noexcept
    : _root(), _error_code(FT_ERR_SUCCESS)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_behavior_tree::~ft_behavior_tree() noexcept
{
    return ;
}

ft_behavior_tree::ft_behavior_tree(const ft_behavior_tree &other) noexcept
    : _root(other._root), _error_code(other._error_code)
{
    this->set_error(other._error_code);
    return ;
}

ft_behavior_tree &ft_behavior_tree::operator=(const ft_behavior_tree &other) noexcept
{
    if (this != &other)
    {
        this->_root = other._root;
        this->set_error(other._error_code);
    }
    return (*this);
}

void ft_behavior_tree::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    return ;
}

void ft_behavior_tree::set_root(const ft_sharedptr<ft_behavior_node> &root) noexcept
{
    this->_root = root;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_sharedptr<ft_behavior_node> &ft_behavior_tree::get_root() noexcept
{
    this->set_error(FT_ERR_SUCCESS);
    return (this->_root);
}

const ft_sharedptr<ft_behavior_node> &ft_behavior_tree::get_root() const noexcept
{
    const_cast<ft_behavior_tree *>(this)->set_error(FT_ERR_SUCCESS);
    return (this->_root);
}

int ft_behavior_tree::tick(ft_behavior_context &context) noexcept
{
    if (!this->_root)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_BEHAVIOR_STATUS_FAILURE);
    }
    int status = this->_root->tick(context);

    this->set_error(this->_root->get_error());
    return (status);
}

int ft_behavior_tree::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_behavior_tree::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
