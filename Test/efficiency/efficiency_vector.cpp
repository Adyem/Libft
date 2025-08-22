#include "../../Template/vector.hpp"
#include "efficiency_utils.hpp"

#include <vector>

int test_efficiency_vector_push_back(void)
{
    const size_t iterations = 20000;
    std::vector<int> stdv;
    ft_vector<int> ftv;
    stdv.reserve(iterations);
    ftv.reserve(iterations);
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&stdv);
        stdv.push_back(static_cast<int>(i));
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&ftv);
        ftv.push_back(static_cast<int>(i));
    }
    auto end_ft = clock_type::now();

    for (size_t i = 0; i < stdv.size(); ++i)
        sum += stdv[i];
    for (size_t i = 0; i < ftv.size(); ++i)
        sum += ftv[i];
    prevent_optimization((void*)&sum);

    print_comparison("vector push_back", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (stdv.size() == ftv.size() ? 1 : 0);
}

int test_efficiency_vector_insert_erase(void)
{
    const size_t base = 1000;
    const size_t iterations = 5000;
    std::vector<int> stdv(base, 1);
    ft_vector<int> ftv;
    ftv.resize(base, 1);

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        size_t pos = stdv.size() / 2;
        stdv.insert(stdv.begin() + pos, static_cast<int>(i));
        stdv.erase(stdv.begin() + pos);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        size_t pos = ftv.size() / 2;
        ftv.insert(ftv.begin() + pos, static_cast<int>(i));
        ftv.erase(ftv.begin() + pos);
    }
    auto end_ft = clock_type::now();

    if (!stdv.empty())
        prevent_optimization(stdv.data());
    if (ftv.size() > 0)
        prevent_optimization(&ftv[0]);

    print_comparison("vector insert/erase", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (stdv.size() == ftv.size() ? 1 : 0);
}

int test_efficiency_vector_reserve_resize(void)
{
    const size_t iterations = 5000;
    std::vector<int> stdv;
    ft_vector<int> ftv;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        stdv.reserve(i);
        stdv.resize(i, 0);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        ftv.reserve(i);
        ftv.resize(i, 0);
    }
    auto end_ft = clock_type::now();

    if (!stdv.empty())
        prevent_optimization(stdv.data());
    if (ftv.size() > 0)
        prevent_optimization(&ftv[0]);

    print_comparison("vector reserve/resize", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (stdv.size() == ftv.size() ? 1 : 0);
}

