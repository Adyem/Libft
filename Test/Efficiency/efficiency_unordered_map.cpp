#include "../../Template/unordered_map.hpp"
#include "../../Template/swap.hpp"
#include "utils.hpp"

#include <unordered_map>
#include <utility>

static void populate_std_map(std::unordered_map<int, int>& map_reference, int count)
{
    int index;

    index = 0;
    while (index < count)
    {
        prevent_optimization((void*)&map_reference);
        map_reference.insert(std::make_pair(index, index));
        index++;
    }
    return ;
}

static void populate_ft_map(ft_unordered_map<int, int>& map_reference, int count)
{
    int index;

    index = 0;
    while (index < count)
    {
        prevent_optimization((void*)&map_reference);
        map_reference.insert(index, index);
        index++;
    }
    return ;
}

int test_efficiency_unordered_map_insert_find(void)
{
    const int iterations = 10000;
    std::unordered_map<int, int> std_map;
    ft_unordered_map<int, int> ft_map;
    long long sum;
    auto start_std = clock_type::now();
    int index;

    sum = 0;
    populate_std_map(std_map, iterations);
    index = 0;
    while (index < iterations)
    {
        std::unordered_map<int, int>::iterator iterator_std = std_map.find(index);
        if (iterator_std != std_map.end())
            sum += iterator_std->second;
        index++;
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    populate_ft_map(ft_map, iterations);
    index = 0;
    while (index < iterations)
    {
        ft_unordered_map<int, int>::iterator iterator_ft = ft_map.find(index);
        if (iterator_ft != ft_map.end())
            sum += iterator_ft->second;
        index++;
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("unordered_map insert/find", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_map.size() == ft_map.size())
        return (1);
    return (0);
}

int test_efficiency_unordered_map_insert_erase(void)
{
    const int iterations = 10000;
    std::unordered_map<int, int> std_map;
    ft_unordered_map<int, int> ft_map;
    int index;

    auto start_std = clock_type::now();
    populate_std_map(std_map, iterations);
    index = 0;
    while (index < iterations)
    {
        std_map.erase(index);
        index++;
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    populate_ft_map(ft_map, iterations);
    index = 0;
    while (index < iterations)
    {
        ft_map.erase(index);
        index++;
    }
    auto end_ft = clock_type::now();

    if (!std_map.empty())
        prevent_optimization((void*)&*std_map.begin());
    if (!ft_map.empty())
        prevent_optimization((void*)&*ft_map.begin());

    print_comparison("unordered_map insert/erase", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_map.empty() == ft_map.empty())
        return (1);
    return (0);
}

int test_efficiency_unordered_map_iterate(void)
{
    const int iterations = 10000;
    std::unordered_map<int, int> std_map;
    ft_unordered_map<int, int> ft_map;
    long long sum;

    sum = 0;
    populate_std_map(std_map, iterations);
    populate_ft_map(ft_map, iterations);

    auto start_std = clock_type::now();
    std::unordered_map<int, int>::iterator iterator_std = std_map.begin();
    while (iterator_std != std_map.end())
    {
        sum += iterator_std->second;
        ++iterator_std;
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ft_unordered_map<int, int>::iterator iterator_ft = ft_map.begin();
    while (iterator_ft != ft_map.end())
    {
        sum += iterator_ft->second;
        ++iterator_ft;
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("unordered_map iterate", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_map.size() == ft_map.size())
        return (1);
    return (0);
}

int test_efficiency_unordered_map_copy(void)
{
    const int iterations = 10000;
    std::unordered_map<int, int> std_source;
    ft_unordered_map<int, int> ft_source;
    long long sum;

    sum = 0;
    populate_std_map(std_source, iterations);
    populate_ft_map(ft_source, iterations);

    auto start_std = clock_type::now();
    std::unordered_map<int, int> std_copy(std_source);
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ft_unordered_map<int, int> ft_copy(ft_source);
    auto end_ft = clock_type::now();

    std::unordered_map<int, int>::iterator iterator_std = std_copy.begin();
    while (iterator_std != std_copy.end())
    {
        sum += iterator_std->second;
        ++iterator_std;
    }
    ft_unordered_map<int, int>::iterator iterator_ft = ft_copy.begin();
    while (iterator_ft != ft_copy.end())
    {
        sum += iterator_ft->second;
        ++iterator_ft;
    }
    prevent_optimization((void*)&sum);

    print_comparison("unordered_map copy", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_copy.size() == ft_copy.size())
        return (1);
    return (0);
}

int test_efficiency_unordered_map_move(void)
{
    const int iterations = 10000;
    std::unordered_map<int, int> std_source;
    ft_unordered_map<int, int> ft_source;
    long long sum;

    sum = 0;
    populate_std_map(std_source, iterations);
    populate_ft_map(ft_source, iterations);

    auto start_std = clock_type::now();
    std::unordered_map<int, int> std_moved(std::move(std_source));
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ft_unordered_map<int, int> ft_moved(std::move(ft_source));
    auto end_ft = clock_type::now();

    std::unordered_map<int, int>::iterator iterator_std = std_moved.begin();
    while (iterator_std != std_moved.end())
    {
        sum += iterator_std->second;
        ++iterator_std;
    }
    ft_unordered_map<int, int>::iterator iterator_ft = ft_moved.begin();
    while (iterator_ft != ft_moved.end())
    {
        sum += iterator_ft->second;
        ++iterator_ft;
    }
    prevent_optimization((void*)&sum);

    print_comparison("unordered_map move", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_moved.size() == ft_moved.size())
        return (1);
    return (0);
}

int test_efficiency_unordered_map_clear(void)
{
    const int elements = 10000;
    std::unordered_map<int, int> std_map;
    ft_unordered_map<int, int> ft_map;
    int index;

    index = 0;
    while (index < elements)
    {
        std_map.insert(std::make_pair(index, index));
        ft_map.insert(index, index);
        index++;
    }

    auto start_std = clock_type::now();
    std_map.clear();
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ft_map.clear();
    auto end_ft = clock_type::now();

    if (!std_map.empty())
        prevent_optimization((void*)&*std_map.begin());
    if (!ft_map.empty())
        prevent_optimization((void*)&*ft_map.begin());

    print_comparison("unordered_map clear", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_map.empty() == ft_map.empty())
        return (1);
    return (0);
}

int test_efficiency_unordered_map_swap(void)
{
    const int elements = 10000;
    const int iterations = 100;
    std::unordered_map<int, int> std_a;
    std::unordered_map<int, int> std_b;
    ft_unordered_map<int, int> ft_a;
    ft_unordered_map<int, int> ft_b;
    int index;

    index = 0;
    while (index < elements)
    {
        std_a.insert(std::make_pair(index, index));
        std_b.insert(std::make_pair(-index, index));
        ft_a.insert(index, index);
        ft_b.insert(-index, index);
        index++;
    }

    auto start_std = clock_type::now();
    index = 0;
    while (index < iterations)
    {
        std::swap(std_a, std_b);
        index++;
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    index = 0;
    while (index < iterations)
    {
        ft_swap(ft_a, ft_b);
        index++;
    }
    auto end_ft = clock_type::now();

    print_comparison("unordered_map swap", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_a.size() == ft_a.size() && std_b.size() == ft_b.size())
        return (1);
    return (0);
}
