#include "../../Template/function.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"

struct counter_callable
{
    mutable int _value;

    counter_callable(int initial_value)
        : _value(initial_value)
    {
        return ;
    }

    counter_callable(const counter_callable &other)
        : _value(other._value)
    {
        return ;
    }

    int operator()() const
    {
        this->_value += 1;
        return (this->_value);
    }
};

FT_TEST(test_ft_function_copy_constructor_clones_state,
        "ft_function copy constructor clones the callable state")
{
    ft_function<int()> source(counter_callable(0));
    ft_function<int()> copied(source);

    FT_ASSERT_EQ(1, source());
    FT_ASSERT_EQ(1, copied());
    FT_ASSERT_EQ(2, source());
    FT_ASSERT_EQ(2, copied());
    return (1);
}

FT_TEST(test_ft_function_copy_assignment_clones_state,
        "ft_function copy assignment clones the callable state")
{
    ft_function<int()> source(counter_callable(5));
    ft_function<int()> destination(counter_callable(0));

    destination = source;
    FT_ASSERT_EQ(6, source());
    FT_ASSERT_EQ(6, destination());
    FT_ASSERT_EQ(7, source());
    FT_ASSERT_EQ(7, destination());
    return (1);
}

FT_TEST(test_ft_function_move_resets_source,
        "ft_function move operations leave the source empty")
{
    ft_function<int()> source(counter_callable(9));
    ft_function<int()> moved(ft_move(source));

    FT_ASSERT_EQ(false, static_cast<bool>(source));
    FT_ASSERT_EQ(10, moved());
    source = ft_move(moved);
    FT_ASSERT_EQ(false, static_cast<bool>(moved));
    FT_ASSERT_EQ(11, source());
    return (1);
}
