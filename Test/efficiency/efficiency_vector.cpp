#include "../../Template/vector.hpp"
#include "../../Template/swap.hpp"
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

int test_efficiency_vector_clear(void)
{
    const size_t elements = 50000;
    std::vector<int> stdv(elements, 1);
    ft_vector<int> ftv;
    ftv.resize(elements, 1);
    volatile long long sum = 0;

    for (size_t i = 0; i < stdv.size(); ++i)
        sum += stdv[i];
    for (size_t i = 0; i < ftv.size(); ++i)
        sum += ftv[i];
    prevent_optimization((void*)&sum);

    auto start_std = clock_type::now();
    stdv.clear();
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ftv.clear();
    auto end_ft = clock_type::now();

    print_comparison("vector clear", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (stdv.empty() == (ftv.size() == 0) ? 1 : 0);
}

int test_efficiency_vector_iterate(void)
{
    const size_t iterations = 50000;
    std::vector<int> stdv(iterations, 1);
    ft_vector<int> ftv;
    ftv.resize(iterations, 1);
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < stdv.size(); ++i)
        sum += stdv[i];
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < ftv.size(); ++i)
        sum += ftv[i];
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("vector iterate", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (stdv.size() == ftv.size() ? 1 : 0);
}

int test_efficiency_vector_move(void)
{
    const size_t elements = 50000;
    std::vector<int> std_src(elements, 1);
    ft_vector<int> ft_src;
    ft_src.resize(elements, 1);
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    std::vector<int> std_moved(std::move(std_src));
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ft_vector<int> ft_moved(std::move(ft_src));
    auto end_ft = clock_type::now();

    for (size_t i = 0; i < std_moved.size(); ++i)
        sum += std_moved[i];
    for (size_t i = 0; i < ft_moved.size(); ++i)
        sum += ft_moved[i];
    prevent_optimization((void*)&sum);

    print_comparison("vector move", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (std_moved.size() == ft_moved.size() ? 1 : 0);
}

int test_efficiency_vector_swap(void)
{
    const size_t elements = 50000;
    const size_t iterations = 1000;
    std::vector<int> std_a(elements, 1), std_b(elements, 2);
    ft_vector<int> ft_a, ft_b;
    ft_a.resize(elements, 1);
    ft_b.resize(elements, 2);
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

    sum += std_a[0] + std_b[0];
    sum += ft_a[0] + ft_b[0];
    prevent_optimization((void*)&sum);

    print_comparison("vector swap", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (std_a.size() == ft_a.size() && std_b.size() == ft_b.size() ? 1 : 0);
}

