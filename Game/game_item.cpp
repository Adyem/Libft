#include "game_item.hpp"

ft_item::ft_item() noexcept
    : _max_stack(0), _stack_size(0), _item_id(0), _rarity(0),
      _width(1), _height(1), _modifier1{0, 0}, _modifier2{0, 0},
      _modifier3{0, 0}, _modifier4{0, 0}, _error_code(ER_SUCCESS)
{
    return ;
}

ft_item::ft_item(const ft_item &other) noexcept
    : _max_stack(other._max_stack), _stack_size(other._stack_size), _item_id(other._item_id), _rarity(other._rarity),
      _width(other._width), _height(other._height), _modifier1(other._modifier1), _modifier2(other._modifier2), _modifier3(other._modifier3), _modifier4(other._modifier4), _error_code(other._error_code)
{
    return ;
}

ft_item &ft_item::operator=(const ft_item &other) noexcept
{
    if (this != &other)
    {
        this->_max_stack = other._max_stack;
        this->_stack_size = other._stack_size;
        this->_item_id = other._item_id;
        this->_rarity = other._rarity;
        this->_width = other._width;
        this->_height = other._height;
        this->_modifier1 = other._modifier1;
        this->_modifier2 = other._modifier2;
        this->_modifier3 = other._modifier3;
        this->_modifier4 = other._modifier4;
        this->_error_code = other._error_code;
    }
    return (*this);
}

ft_item::ft_item(ft_item &&other) noexcept
    : _max_stack(other._max_stack), _stack_size(other._stack_size), _item_id(other._item_id), _rarity(other._rarity),
      _width(other._width), _height(other._height), _modifier1(other._modifier1), _modifier2(other._modifier2), _modifier3(other._modifier3), _modifier4(other._modifier4), _error_code(other._error_code)
{
    other._max_stack = 0;
    other._stack_size = 0;
    other._item_id = 0;
    other._rarity = 0;
    other._width = 1;
    other._height = 1;
    other._modifier1.id = 0;
    other._modifier1.value = 0;
    other._modifier2.id = 0;
    other._modifier2.value = 0;
    other._modifier3.id = 0;
    other._modifier3.value = 0;
    other._modifier4.id = 0;
    other._modifier4.value = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

ft_item &ft_item::operator=(ft_item &&other) noexcept
{
    if (this != &other)
    {
        this->_max_stack = other._max_stack;
        this->_stack_size = other._stack_size;
        this->_item_id = other._item_id;
        this->_rarity = other._rarity;
        this->_width = other._width;
        this->_height = other._height;
        this->_modifier1 = other._modifier1;
        this->_modifier2 = other._modifier2;
        this->_modifier3 = other._modifier3;
        this->_modifier4 = other._modifier4;
        this->_error_code = other._error_code;
        other._max_stack = 0;
        other._stack_size = 0;
        other._item_id = 0;
        other._rarity = 0;
        other._width = 1;
        other._height = 1;
        other._modifier1.id = 0;
        other._modifier1.value = 0;
        other._modifier2.id = 0;
        other._modifier2.value = 0;
        other._modifier3.id = 0;
        other._modifier3.value = 0;
        other._modifier4.id = 0;
        other._modifier4.value = 0;
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

int ft_item::get_max_stack() const noexcept
{
    return (this->_max_stack);
}

void ft_item::set_max_stack(int max) noexcept
{
    this->_max_stack = max;
    return ;
}

int ft_item::get_stack_size() const noexcept
{
    return (this->_stack_size);
}

void ft_item::set_stack_size(int amount) noexcept
{
    this->_stack_size = amount;
    return ;
}

void ft_item::add_to_stack(int amount) noexcept
{
    this->_stack_size += amount;
    if (this->_stack_size > this->_max_stack)
        this->_stack_size = this->_max_stack;
    return ;
}

void ft_item::sub_from_stack(int amount) noexcept
{
    this->_stack_size -= amount;
    if (this->_stack_size < 0)
        this->_stack_size = 0;
    return ;
}

int ft_item::get_item_id() const noexcept
{
    return (this->_item_id);
}

void ft_item::set_item_id(int id) noexcept
{
    this->_item_id = id;
    return ;
}

int ft_item::get_width() const noexcept
{
    return (this->_width);
}

void ft_item::set_width(int width) noexcept
{
    this->_width = width;
    return ;
}

int ft_item::get_height() const noexcept
{
    return (this->_height);
}

void ft_item::set_height(int height) noexcept
{
    this->_height = height;
    return ;
}

int ft_item::get_rarity() const noexcept
{
    return (this->_rarity);
}

void ft_item::set_rarity(int rarity) noexcept
{
    this->_rarity = rarity;
    return ;
}

ft_item_modifier ft_item::get_modifier1() const noexcept
{
    return (this->_modifier1);
}

void ft_item::set_modifier1(const ft_item_modifier &mod) noexcept
{
    this->_modifier1 = mod;
    return ;
}

int ft_item::get_modifier1_id() const noexcept
{
    return (this->_modifier1.id);
}

void ft_item::set_modifier1_id(int id) noexcept
{
    this->_modifier1.id = id;
    return ;
}

int ft_item::get_modifier1_value() const noexcept
{
    return (this->_modifier1.value);
}

void ft_item::set_modifier1_value(int value) noexcept
{
    this->_modifier1.value = value;
    return ;
}

ft_item_modifier ft_item::get_modifier2() const noexcept
{
    return (this->_modifier2);
}

void ft_item::set_modifier2(const ft_item_modifier &mod) noexcept
{
    this->_modifier2 = mod;
    return ;
}

int ft_item::get_modifier2_id() const noexcept
{
    return (this->_modifier2.id);
}

void ft_item::set_modifier2_id(int id) noexcept
{
    this->_modifier2.id = id;
    return ;
}

int ft_item::get_modifier2_value() const noexcept
{
    return (this->_modifier2.value);
}

void ft_item::set_modifier2_value(int value) noexcept
{
    this->_modifier2.value = value;
    return ;
}

ft_item_modifier ft_item::get_modifier3() const noexcept
{
    return (this->_modifier3);
}

void ft_item::set_modifier3(const ft_item_modifier &mod) noexcept
{
    this->_modifier3 = mod;
    return ;
}

int ft_item::get_modifier3_id() const noexcept
{
    return (this->_modifier3.id);
}

void ft_item::set_modifier3_id(int id) noexcept
{
    this->_modifier3.id = id;
    return ;
}

int ft_item::get_modifier3_value() const noexcept
{
    return (this->_modifier3.value);
}

void ft_item::set_modifier3_value(int value) noexcept
{
    this->_modifier3.value = value;
    return ;
}

ft_item_modifier ft_item::get_modifier4() const noexcept
{
    return (this->_modifier4);
}

void ft_item::set_modifier4(const ft_item_modifier &mod) noexcept
{
    this->_modifier4 = mod;
    return ;
}

int ft_item::get_modifier4_id() const noexcept
{
    return (this->_modifier4.id);
}

void ft_item::set_modifier4_id(int id) noexcept
{
    this->_modifier4.id = id;
    return ;
}

int ft_item::get_modifier4_value() const noexcept
{
    return (this->_modifier4.value);
}

void ft_item::set_modifier4_value(int value) noexcept
{
    this->_modifier4.value = value;
    return ;
}

void ft_item::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error_code = err;
    return ;
}

int ft_item::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_item::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

