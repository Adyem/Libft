#include "../../Template/stack.hpp"
#include "../../Template/swap.hpp"
#include "efficiency_utils.hpp"

#include <stack>

int test_efficiency_stack_push_pop(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    {
        std::stack<int> st;
        for (size_t i = 0; i < iterations; ++i)
            st.push(static_cast<int>(i));
        while (!st.empty())
        {
            sum += st.top();
            st.pop();
        }
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_stack<int> st;
        for (size_t i = 0; i < iterations; ++i)
            st.push(static_cast<int>(i));
        while (st.size() > 0)
        {
            sum += st.top();
            st.pop();
        }
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("stack push/pop", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_stack_interleaved(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    {
        std::stack<int> st;
        for (size_t i = 0; i < iterations; ++i)
        {
            st.push(static_cast<int>(i));
            sum += st.top();
            st.pop();
        }
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_stack<int> st;
        for (size_t i = 0; i < iterations; ++i)
        {
            st.push(static_cast<int>(i));
            sum += st.top();
            st.pop();
        }
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("stack interleaved", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_stack_move(void)
{
    const size_t elements = 100000;
    std::stack<int> std_src;
    ft_stack<int> ft_src;
    for (size_t i = 0; i < elements; ++i)
    {
        std_src.push(static_cast<int>(i));
        ft_src.push(static_cast<int>(i));
    }
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    std::stack<int> std_moved(std::move(std_src));
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ft_stack<int> ft_moved(std::move(ft_src));
    auto end_ft = clock_type::now();

    while (!std_moved.empty())
    {
        sum += std_moved.top();
        std_moved.pop();
    }
    while (ft_moved.size() > 0)
    {
        sum += ft_moved.top();
        ft_moved.pop();
    }
    prevent_optimization((void*)&sum);

    print_comparison("stack move", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_stack_swap(void)
{
    const size_t elements = 100000;
    const size_t iterations = 1000;
    std::stack<int> std_a, std_b;
    ft_stack<int> ft_a, ft_b;

    for (size_t i = 0; i < elements; ++i)
    {
        std_a.push(static_cast<int>(i));
        std_b.push(static_cast<int>(elements - i));
        ft_a.push(static_cast<int>(i));
        ft_b.push(static_cast<int>(elements - i));
    }
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&std_a);
        prevent_optimization((void*)&std_b);
        std::swap(std_a, std_b);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&ft_a);
        prevent_optimization((void*)&ft_b);
        ft_swap(ft_a, ft_b);
    }
    auto end_ft = clock_type::now();

    sum += std_a.top();
    sum += std_b.top();
    sum += ft_a.top();
    sum += ft_b.top();
    prevent_optimization((void*)&sum);

    print_comparison("stack swap", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (std_a.size() == ft_a.size() && std_b.size() == ft_b.size() ? 1 : 0);
}

