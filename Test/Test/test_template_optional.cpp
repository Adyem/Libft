#include "../test_internal.hpp"
#include "../../Template/optional.hpp"
#include "../../Template/optional.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include <utility>

#ifndef LIBFT_TEST_BUILD
#endif

class no_default_optional_value
{
    private:
        int _value;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        no_default_optional_value(int value);
        no_default_optional_value(const no_default_optional_value &other);
        no_default_optional_value &operator=(const no_default_optional_value &other);
        ~no_default_optional_value();

        int get_value() const;
        int get_error() const;
        const char *get_error_str() const;
};

no_default_optional_value::no_default_optional_value(int value)
    : _value(value), _error_code(FT_ERR_SUCCESS)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

no_default_optional_value::no_default_optional_value(const no_default_optional_value &other)
    : _value(other._value), _error_code(other._error_code)
{
    this->set_error(other._error_code);
    return ;
}

no_default_optional_value &no_default_optional_value::operator=(const no_default_optional_value &other)
{
    if (this != &other)
    {
        this->_value = other._value;
        this->_error_code = other._error_code;
        this->set_error(other._error_code);
    }
    return (*this);
}

no_default_optional_value::~no_default_optional_value()
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void no_default_optional_value::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

int no_default_optional_value::get_value() const
{
    this->set_error(FT_ERR_SUCCESS);
    return (this->_value);
}

int no_default_optional_value::get_error() const
{
    this->set_error(this->_error_code);
    return (this->_error_code);
}

const char *no_default_optional_value::get_error_str() const
{
    this->set_error(this->_error_code);
    return (ft_strerror(this->_error_code));
}

FT_TEST(test_ft_optional_non_default_constructible, "ft_optional stores and retrieves non-default-constructible values")
{
    ft_optional<no_default_optional_value> empty_optional;
    ft_global_error_stack_pop_all();
    FT_ASSERT(empty_optional.has_value() == false);
    empty_optional.value();
    FT_ASSERT_EQ(FT_ERR_EMPTY, ft_global_error_stack_peek_last_error());
    ft_global_error_stack_pop_last();

    ft_optional<no_default_optional_value> value_optional(no_default_optional_value(42));
    FT_ASSERT(value_optional.has_value() == true);
    FT_ASSERT_EQ(42, value_optional.value().get_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_global_error_stack_peek_last_error());
    ft_global_error_stack_pop_last();

    ft_optional<no_default_optional_value> moved_optional(no_default_optional_value(64));
    value_optional = std::move(moved_optional);
    FT_ASSERT(value_optional.has_value() == true);
    FT_ASSERT_EQ(64, value_optional.value().get_value());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_global_error_stack_peek_last_error());
    ft_global_error_stack_pop_last();
    FT_ASSERT(moved_optional.has_value() == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_global_error_stack_peek_last_error());
    ft_global_error_stack_pop_last();

    value_optional.reset();
    FT_ASSERT(value_optional.has_value() == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_global_error_stack_peek_last_error());
    ft_global_error_stack_pop_last();
    return (1);
}
