#include "../../Template/template_unordened_map.hpp"
#include "../../Template/template_swap.hpp"
#include "efficiency_utils.hpp"

#include <unordered_map>

int test_efficiency_unord_map_insert_find(void)
{
    const int iterations = 10000;
    std::unordered_map<int, int> stdm;
    ft_unord_map<int, int> ftm;
    long long sum = 0;

    auto start_std = clock_type::now();
    for (int i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&stdm);
        stdm.insert(std::make_pair(i, i));
    }
    for (int i = 0; i < iterations; ++i)
    {
        std::unordered_map<int, int>::iterator it = stdm.find(i);
        if (it != stdm.end())
            sum += it->second;
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (int i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&ftm);
        ftm.insert(i, i);
    }
    for (int i = 0; i < iterations; ++i)
    {
        ft_unord_map<int, int>::iterator it = ftm.find(i);
        if (it != ftm.end())
            sum += it->second;
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("unord_map insert/find", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (stdm.size() == ftm.getSize())
        return (1);
    return (0);
}

int test_efficiency_unord_map_insert_remove(void)
{
    const int iterations = 10000;
    std::unordered_map<int, int> stdm;
    ft_unord_map<int, int> ftm;

    auto start_std = clock_type::now();
    for (int i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&stdm);
        stdm.insert(std::make_pair(i, i));
    }
    for (int i = 0; i < iterations; ++i)
        stdm.erase(i);
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (int i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&ftm);
        ftm.insert(i, i);
    }
    for (int i = 0; i < iterations; ++i)
        ftm.remove(i);
    auto end_ft = clock_type::now();

    if (!stdm.empty())
        prevent_optimization((void*)&*stdm.begin());
    if (!ftm.empty())
        prevent_optimization((void*)&*ftm.begin());

    print_comparison("unord_map insert/remove", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (stdm.empty() == ftm.empty())
        return (1);
    return (0);
}

int test_efficiency_unord_map_iterate(void)
{
    const int iterations = 10000;
    std::unordered_map<int, int> stdm;
    ft_unord_map<int, int> ftm;
    long long sum = 0;

    for (int i = 0; i < iterations; ++i)
        stdm.insert(std::make_pair(i, i));
    for (int i = 0; i < iterations; ++i)
        ftm.insert(i, i);

    auto start_std = clock_type::now();
    for (std::unordered_map<int, int>::iterator it = stdm.begin(); it != stdm.end(); ++it)
        sum += it->second;
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (ft_unord_map<int, int>::iterator it = ftm.begin(); it != ftm.end(); ++it)
        sum += it->second;
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("unord_map iterate", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (stdm.size() == ftm.getSize())
        return (1);
    return (0);
}

int test_efficiency_unord_map_copy(void)
{
    const int iterations = 10000;
    std::unordered_map<int, int> std_src;
    ft_unord_map<int, int> ft_src;
    long long sum = 0;

    for (int i = 0; i < iterations; ++i)
    {
        std_src.insert(std::make_pair(i, i));
        ft_src.insert(i, i);
    }

    auto start_std = clock_type::now();
    std::unordered_map<int, int> std_copy(std_src);
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ft_unord_map<int, int> ft_copy(ft_src);
    auto end_ft = clock_type::now();

    for (std::unordered_map<int, int>::iterator it = std_copy.begin(); it != std_copy.end(); ++it)
        sum += it->second;
    for (ft_unord_map<int, int>::iterator it = ft_copy.begin(); it != ft_copy.end(); ++it)
        sum += it->second;
    prevent_optimization((void*)&sum);

    print_comparison("unord_map copy", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_copy.size() == ft_copy.getSize())
        return (1);
    return (0);
}

int test_efficiency_unord_map_move(void)
{
    const int iterations = 10000;
    std::unordered_map<int, int> std_src;
    ft_unord_map<int, int> ft_src;
    long long sum = 0;

    for (int i = 0; i < iterations; ++i)
    {
        std_src.insert(std::make_pair(i, i));
        ft_src.insert(i, i);
    }

    auto start_std = clock_type::now();
    std::unordered_map<int, int> std_moved(std::move(std_src));
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ft_unord_map<int, int> ft_moved(std::move(ft_src));
    auto end_ft = clock_type::now();

    for (std::unordered_map<int, int>::iterator it = std_moved.begin(); it != std_moved.end(); ++it)
        sum += it->second;
    for (ft_unord_map<int, int>::iterator it = ft_moved.begin(); it != ft_moved.end(); ++it)
        sum += it->second;
    prevent_optimization((void*)&sum);

    print_comparison("unord_map move", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_moved.size() == ft_moved.getSize())
        return (1);
    return (0);
}

int test_efficiency_unord_map_swap(void)
{
    const int elements = 10000;
    const int iterations = 100;
    std::unordered_map<int, int> std_a, std_b;
    ft_unord_map<int, int> ft_a, ft_b;

    for (int i = 0; i < elements; ++i)
    {
        std_a.insert(std::make_pair(i, i));
        std_b.insert(std::make_pair(-i, i));
        ft_a.insert(i, i);
        ft_b.insert(-i, i);
    }
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    for (int i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&std_a);
        prevent_optimization((void*)&std_b);
        std::swap(std_a, std_b);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (int i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&ft_a);
        prevent_optimization((void*)&ft_b);
        ft_swap(ft_a, ft_b);
    }
    auto end_ft = clock_type::now();

    sum += std_a.begin()->second;
    sum += std_b.begin()->second;
    sum += ft_a.begin()->second;
    sum += ft_b.begin()->second;
    prevent_optimization((void*)&sum);

    print_comparison("unord_map swap", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_a.size() == ft_a.getSize() && std_b.size() == ft_b.getSize())
        return (1);
    return (0);
}

int test_efficiency_unord_map_clear(void)
{
    const int elements = 10000;
    std::unordered_map<int, int> stdm;
    ft_unord_map<int, int> ftm;

    for (int i = 0; i < elements; ++i)
    {
        stdm.insert(std::make_pair(i, i));
        ftm.insert(i, i);
    }

    auto start_std = clock_type::now();
    stdm.clear();
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ftm.clear();
    auto end_ft = clock_type::now();

    if (!stdm.empty())
        prevent_optimization((void*)&*stdm.begin());
    if (!ftm.empty())
        prevent_optimization((void*)&*ftm.begin());

    print_comparison("unord_map clear", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (stdm.empty() == ftm.empty())
        return (1);
    return (0);
}

