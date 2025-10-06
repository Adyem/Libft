#include "../../CPP_class/class_string_class.hpp"
#include "utils.hpp"

#include <string>
#include <utility>

int test_efficiency_string_create(void)
{
    const size_t iterations = 100000;
    const char *cstr = "benchmark";
    volatile size_t sum = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        std::string s(cstr);
        prevent_optimization((void*)s.c_str());
        sum += s.size();
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        ft_string s(cstr);
        prevent_optimization((void*)s.c_str());
        sum += s.size();
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("string create", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_string_copy(void)
{
    const size_t iterations = 100000;
    const char *cstr = "benchmark";
    volatile size_t sum = 0;

    auto start_std = clock_type::now();
    {
        std::string base(cstr);
        for (size_t i = 0; i < iterations; ++i)
        {
            std::string copy(base);
            prevent_optimization((void*)copy.c_str());
            sum += copy.size();
        }
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_string base(cstr);
        for (size_t i = 0; i < iterations; ++i)
        {
            ft_string copy(base);
            prevent_optimization((void*)copy.c_str());
            sum += copy.size();
        }
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("string copy", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_string_move(void)
{
    const size_t iterations = 100000;
    volatile size_t sum = 0;

    auto start_std = clock_type::now();
    {
        std::string s("benchmark");
        for (size_t i = 0; i < iterations; ++i)
        {
            std::string mv = std::move(s);
            prevent_optimization((void*)mv.c_str());
            sum += mv.size();
            s = std::move(mv);
        }
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_string s("benchmark");
        for (size_t i = 0; i < iterations; ++i)
        {
            ft_string mv = std::move(s);
            prevent_optimization((void*)mv.c_str());
            sum += mv.size();
            s = std::move(mv);
        }
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("string move", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_string_append(void)
{
    const size_t iterations = 50000;
    volatile size_t sum = 0;

    auto start_std = clock_type::now();
    {
        std::string s;
        for (size_t i = 0; i < iterations; ++i)
            s += 'a';
        sum += s.size();
        prevent_optimization((void*)s.c_str());
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_string s;
        for (size_t i = 0; i < iterations; ++i)
            s.append('a');
        sum += s.size();
        prevent_optimization((void*)s.c_str());
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("string append", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

