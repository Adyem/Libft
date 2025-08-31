#include "../../Template/map.hpp"
#include "../../Template/swap.hpp"
#include "efficiency_utils.hpp"

#include <map>

int test_efficiency_map_insert_find(void)
{
    const int iterations = 10000;
    std::map<int, int> stdm;
    ft_map<int, int> ftm;
    long long sum = 0;

    auto start_std = clock_type::now();
    for (int i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&stdm);
        stdm.insert(std::make_pair(i, i));
    }
    for (int i = 0; i < iterations; ++i)
    {
        std::map<int, int>::iterator it = stdm.find(i);
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
        Pair<int, int>* p = ftm.find(i);
        if (p)
            sum += p->value;
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);

    print_comparison("map insert/find", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (stdm.size() == ftm.getSize())
        return (1);
    return (0);
}

int test_efficiency_map_insert_remove(void)
{
    const int iterations = 10000;
    std::map<int, int> stdm;
    ft_map<int, int> ftm;

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
        prevent_optimization((void*)ftm.find(0));

    print_comparison("map insert/remove", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (stdm.empty() == ftm.empty())
        return (1);
    return (0);
}

int test_efficiency_map_iterate(void)
{
    const int iterations = 10000;
    std::map<int, int> stdm;
    ft_map<int, int> ftm;
    long long sum = 0;

    for (int i = 0; i < iterations; ++i)
    {
        stdm.insert(std::make_pair(i, i));
        ftm.insert(i, i);
    }

    auto start_std = clock_type::now();
    for (std::map<int, int>::iterator it = stdm.begin(); it != stdm.end(); ++it)
        sum += it->second;
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (Pair<int, int>* it = ftm.find(0); it != ftm.end(); ++it)
        sum += it->value;
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("map iterate", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (stdm.size() == ftm.getSize())
        return (1);
    return (0);
}

int test_efficiency_map_copy(void)
{
    const int iterations = 10000;
    std::map<int, int> std_src;
    ft_map<int, int> ft_src;
    long long sum = 0;

    for (int i = 0; i < iterations; ++i)
    {
        std_src.insert(std::make_pair(i, i));
        ft_src.insert(i, i);
    }

    auto start_std = clock_type::now();
    std::map<int, int> std_copy(std_src);
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ft_map<int, int> ft_copy(ft_src);
    auto end_ft = clock_type::now();

    for (std::map<int, int>::iterator it = std_copy.begin(); it != std_copy.end(); ++it)
        sum += it->second;
    for (Pair<int, int>* it = ft_copy.find(0); it != ft_copy.end(); ++it)
        sum += it->value;
    prevent_optimization((void*)&sum);

    print_comparison("map copy", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_copy.size() == ft_copy.getSize())
        return (1);
    return (0);
}

int test_efficiency_map_move(void)
{
    const int iterations = 10000;
    std::map<int, int> std_src;
    ft_map<int, int> ft_src;
    long long sum = 0;

    for (int i = 0; i < iterations; ++i)
    {
        std_src.insert(std::make_pair(i, i));
        ft_src.insert(i, i);
    }

    auto start_std = clock_type::now();
    std::map<int, int> std_moved(std::move(std_src));
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ft_map<int, int> ft_moved(std::move(ft_src));
    auto end_ft = clock_type::now();

    for (std::map<int, int>::iterator it = std_moved.begin(); it != std_moved.end(); ++it)
        sum += it->second;
    for (Pair<int, int>* it = ft_moved.find(0); it != ft_moved.end(); ++it)
        sum += it->value;
    prevent_optimization((void*)&sum);

    print_comparison("map move", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_moved.size() == ft_moved.getSize())
        return (1);
    return (0);
}

int test_efficiency_map_swap(void)
{
    const int elements = 10000;
    const int iterations = 100;
    std::map<int, int> std_a, std_b;
    ft_map<int, int> ft_a, ft_b;

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
    sum += ft_a.find(0)->value;
    sum += ft_b.find(0)->value;
    prevent_optimization((void*)&sum);

    print_comparison("map swap", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_a.size() == ft_a.getSize() && std_b.size() == ft_b.getSize())
        return (1);
    return (0);
}

int test_efficiency_map_clear(void)
{
    const int elements = 10000;
    std::map<int, int> stdm;
    ft_map<int, int> ftm;

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
        prevent_optimization((void*)ftm.find(0));

    print_comparison("map clear", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (stdm.empty() == ftm.empty())
        return (1);
    return (0);
}

