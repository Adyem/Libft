#include "../../Geometry/geometry.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

FT_TEST(test_intersect_aabb_overlap, "intersect_aabb detects overlapping boxes")
{
    aabb first;
    aabb second;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, first.set_bounds(0.0, 0.0, 5.0, 5.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, second.set_bounds(3.0, 3.0, 8.0, 8.0));
    FT_ASSERT(intersect_aabb(first, second));
    FT_ASSERT(intersect_aabb(second, first));
    return (1);
}

FT_TEST(test_intersect_aabb_separated, "intersect_aabb returns false when separated on axis")
{
    aabb first;
    aabb second;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, first.set_bounds(-2.0, -2.0, 2.0, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, second.set_bounds(3.0, -1.0, 6.0, 1.0));
    FT_ASSERT_EQ(false, intersect_aabb(first, second));
    FT_ASSERT_EQ(false, intersect_aabb(second, first));
    return (1);
}

FT_TEST(test_intersect_aabb_touching_edge, "intersect_aabb treats shared boundary as collision")
{
    aabb first;
    aabb second;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, first.set_bounds(0.0, 0.0, 4.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, second.set_bounds(4.0, 1.0, 7.0, 3.0));
    FT_ASSERT(intersect_aabb(first, second));
    FT_ASSERT(intersect_aabb(second, first));
    return (1);
}

FT_TEST(test_intersect_aabb_parallel_access, "intersect_aabb handles concurrent calls")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_ready;
    std::atomic<bool> worker_failed;
    std::atomic<bool> worker_completed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, first.set_bounds(0.0, 0.0, 10.0, 10.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, second.set_bounds(2.0, 2.0, 8.0, 8.0));

    worker_ready.store(false);
    worker_failed.store(false);
    worker_completed.store(false);

    worker_thread = std::thread([&first, &second, &worker_ready, &worker_failed,
                &worker_completed]() {
        int local_iteration_index;

        ft_errno = FT_ERR_CONFIGURATION;
        worker_ready.store(true);
        local_iteration_index = 0;
        while (local_iteration_index < 256 && !worker_failed.load())
        {
            if (!intersect_aabb(first, second))
            {
                worker_failed.store(true);
                break;
            }
            if (first.get_error() != FT_ERR_SUCCESSS)
            {
                worker_failed.store(true);
                break;
            }
            if (second.get_error() != FT_ERR_SUCCESSS)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        worker_completed.store(true);
        return ;
    });

    while (!worker_ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    ft_errno = FT_ERR_ALREADY_EXISTS;
    iteration_index = 0;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    while (iteration_index < 256)
    {
        if (!intersect_aabb(second, first) && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "intersect_aabb(second, first)";
            failure_line = __LINE__;
            break;
        }
        if (first.get_error() != FT_ERR_SUCCESSS && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "first.get_error() == FT_ERR_SUCCESSS";
            failure_line = __LINE__;
            break;
        }
        if (second.get_error() != FT_ERR_SUCCESSS && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "second.get_error() == FT_ERR_SUCCESSS";
            failure_line = __LINE__;
            break;
        }
        iteration_index = iteration_index + 1;
    }

    worker_thread.join();

    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }

    FT_ASSERT(worker_completed.load());
    FT_ASSERT(worker_failed.load() == false);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, first.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, second.get_error());
    return (1);
}

FT_TEST(test_intersect_aabb_self_reference, "intersect_aabb supports self collisions")
{
    aabb box;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, box.set_bounds(-1.0, -2.0, 3.0, 4.0));
    ft_errno = FT_ERR_ALREADY_EXISTS;
    FT_ASSERT(intersect_aabb(box, box));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, box.get_error());
    FT_ASSERT_EQ(FT_ERR_ALREADY_EXISTS, ft_errno);
    return (1);
}

FT_TEST(test_aabb_set_bounds_invalid, "aabb rejects invalid bounds and preserves state")
{
    aabb box;
    double stored_minimum_x;
    double stored_minimum_y;
    double stored_maximum_x;
    double stored_maximum_y;

    FT_ASSERT_EQ(FT_ERR_SUCCESSS, box.set_bounds(-2.0, -3.0, 4.0, 6.0));
    ft_errno = FT_ERR_CONFIGURATION;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, box.set_bounds(8.0, -3.0, 4.0, 6.0));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, box.get_error());
    stored_minimum_x = box.get_minimum_x();
    stored_minimum_y = box.get_minimum_y();
    stored_maximum_x = box.get_maximum_x();
    stored_maximum_y = box.get_maximum_y();
    FT_ASSERT_EQ(-2.0, stored_minimum_x);
    FT_ASSERT_EQ(-3.0, stored_minimum_y);
    FT_ASSERT_EQ(4.0, stored_maximum_x);
    FT_ASSERT_EQ(6.0, stored_maximum_y);
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, ft_errno);
    return (1);
}
