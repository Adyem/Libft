#include "../../Template/shared_ptr.hpp"
#include "efficiency_utils.hpp"

#include <memory>

int test_efficiency_shared_ptr_create(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        std::shared_ptr<int> sp = std::make_shared<int>(static_cast<int>(i));
        prevent_optimization(sp.get());
        sum += *sp;
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        ft_sharedptr<int> sp(new int(static_cast<int>(i)));
        prevent_optimization(sp.get());
        sum += *sp;
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("shared_ptr create", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_shared_ptr_copy(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    {
        std::shared_ptr<int> sp = std::make_shared<int>(42);
        for (size_t i = 0; i < iterations; ++i)
        {
            std::shared_ptr<int> cp = sp;
            prevent_optimization(cp.get());
            sum += *cp;
        }
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_sharedptr<int> sp(new int(42));
        for (size_t i = 0; i < iterations; ++i)
        {
            ft_sharedptr<int> cp = sp;
            prevent_optimization(cp.get());
            sum += *cp;
        }
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("shared_ptr copy", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_shared_ptr_move(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    {
        std::shared_ptr<int> sp = std::make_shared<int>(42);
        for (size_t i = 0; i < iterations; ++i)
        {
            std::shared_ptr<int> mv = std::move(sp);
            prevent_optimization(mv.get());
            sum += *mv;
            sp = std::move(mv);
        }
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_sharedptr<int> sp(new int(42));
        for (size_t i = 0; i < iterations; ++i)
        {
            ft_sharedptr<int> mv = std::move(sp);
            prevent_optimization(mv.get());
            sum += *mv;
            sp = std::move(mv);
        }
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("shared_ptr move", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_shared_ptr_reset(void)
{
    const size_t iterations = 100000;

    auto start_std = clock_type::now();
    {
        std::shared_ptr<int> sp = std::make_shared<int>(0);
        for (size_t i = 0; i < iterations; ++i)
        {
            sp.reset(new int(static_cast<int>(i)));
            prevent_optimization(sp.get());
        }
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_sharedptr<int> sp(new int(0));
        for (size_t i = 0; i < iterations; ++i)
        {
            sp.reset(new int(static_cast<int>(i)));
            prevent_optimization(sp.get());
        }
    }
    auto end_ft = clock_type::now();

    print_comparison("shared_ptr reset", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_shared_ptr_use_count(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    {
        std::shared_ptr<int> sp = std::make_shared<int>(42);
        std::shared_ptr<int> other = sp;
        for (size_t i = 0; i < iterations; ++i)
            sum += sp.use_count();
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_sharedptr<int> sp(new int(42));
        ft_sharedptr<int> other = sp;
        for (size_t i = 0; i < iterations; ++i)
            sum += sp.use_count();
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("shared_ptr use_count", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_shared_ptr_swap(void)
{
    const size_t iterations = 100000;

    auto start_std = clock_type::now();
    {
        std::shared_ptr<int> a = std::make_shared<int>(1);
        std::shared_ptr<int> b = std::make_shared<int>(2);
        for (size_t i = 0; i < iterations; ++i)
        {
            a.swap(b);
            prevent_optimization(a.get());
            prevent_optimization(b.get());
        }
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_sharedptr<int> a(new int(1));
        ft_sharedptr<int> b(new int(2));
        for (size_t i = 0; i < iterations; ++i)
        {
            a.swap(b);
            prevent_optimization(a.get());
            prevent_optimization(b.get());
        }
    }
    auto end_ft = clock_type::now();

    print_comparison("shared_ptr swap", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

