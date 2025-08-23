#include "../../Template/map.hpp"
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
    return (stdm.size() == ftm.getSize() ? 1 : 0);
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
    return (stdm.empty() == ftm.empty() ? 1 : 0);
}

