#include "../../Geometry/geometry.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>
#include <utility>

FT_TEST(test_intersect_sphere_overlap, "intersect_sphere detects overlapping spheres")
{
    sphere first;
    sphere second;

    FT_ASSERT_EQ(ER_SUCCESS, first.set_center(1.0, 2.0, 3.0));
    FT_ASSERT_EQ(ER_SUCCESS, first.set_radius(5.0));
    FT_ASSERT_EQ(ER_SUCCESS, second.set_center(4.0, 3.0, 5.0));
    FT_ASSERT_EQ(ER_SUCCESS, second.set_radius(4.0));
    FT_ASSERT(intersect_sphere(first, second));
    FT_ASSERT(intersect_sphere(second, first));
    return (1);
}

FT_TEST(test_intersect_sphere_separated, "intersect_sphere returns false when centers far apart")
{
    sphere first;
    sphere second;

    FT_ASSERT_EQ(ER_SUCCESS, first.set_center(-4.0, -4.0, -4.0));
    FT_ASSERT_EQ(ER_SUCCESS, first.set_radius(1.0));
    FT_ASSERT_EQ(ER_SUCCESS, second.set_center(4.0, 4.0, 4.0));
    FT_ASSERT_EQ(ER_SUCCESS, second.set_radius(1.0));
    FT_ASSERT_EQ(false, intersect_sphere(first, second));
    FT_ASSERT_EQ(false, intersect_sphere(second, first));
    return (1);
}

FT_TEST(test_intersect_sphere_touching, "intersect_sphere treats tangential contact as intersection")
{
    sphere first;
    sphere second;

    FT_ASSERT_EQ(ER_SUCCESS, first.set_center(0.0, 0.0, 0.0));
    FT_ASSERT_EQ(ER_SUCCESS, first.set_radius(2.5));
    FT_ASSERT_EQ(ER_SUCCESS, second.set_center(5.0, 0.0, 0.0));
    FT_ASSERT_EQ(ER_SUCCESS, second.set_radius(2.5));
    FT_ASSERT(intersect_sphere(first, second));
    FT_ASSERT(intersect_sphere(second, first));
    return (1);
}

FT_TEST(test_intersect_sphere_parallel_access, "intersect_sphere handles concurrent calls")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_ready;
    std::atomic<bool> worker_failed;
    std::atomic<bool> worker_completed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(ER_SUCCESS, first.set_center(0.0, 0.0, 0.0));
    FT_ASSERT_EQ(ER_SUCCESS, first.set_radius(6.0));
    FT_ASSERT_EQ(ER_SUCCESS, second.set_center(2.0, 2.0, 2.0));
    FT_ASSERT_EQ(ER_SUCCESS, second.set_radius(5.0));

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
            if (!intersect_sphere(first, second))
            {
                worker_failed.store(true);
                break;
            }
            if (first.get_error() != ER_SUCCESS)
            {
                worker_failed.store(true);
                break;
            }
            if (second.get_error() != ER_SUCCESS)
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
        if (!intersect_sphere(second, first) && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "intersect_sphere(second, first)";
            failure_line = __LINE__;
            break;
        }
        if (first.get_error() != ER_SUCCESS && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "first.get_error() == ER_SUCCESS";
            failure_line = __LINE__;
            break;
        }
        if (second.get_error() != ER_SUCCESS && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "second.get_error() == ER_SUCCESS";
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
    FT_ASSERT_EQ(ER_SUCCESS, first.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, second.get_error());
    return (1);
}

FT_TEST(test_intersect_sphere_self_reference, "intersect_sphere supports self collisions")
{
    sphere shape;

    FT_ASSERT_EQ(ER_SUCCESS, shape.set_center(1.0, -2.0, 3.5));
    FT_ASSERT_EQ(ER_SUCCESS, shape.set_radius(4.5));
    ft_errno = FT_ERR_ALREADY_EXISTS;
    FT_ASSERT(intersect_sphere(shape, shape));
    FT_ASSERT_EQ(ER_SUCCESS, shape.get_error());
    FT_ASSERT_EQ(FT_ERR_ALREADY_EXISTS, ft_errno);
    return (1);
}

FT_TEST(test_sphere_move_assignment_resets_source, "sphere move assignment transfers state")
{
    sphere source;
    sphere destination;
    double destination_center_x;
    double destination_center_y;
    double destination_center_z;
    double destination_radius;
    double source_center_x;
    double source_center_y;
    double source_center_z;
    double source_radius;

    FT_ASSERT_EQ(ER_SUCCESS, source.set_center(7.0, -3.0, 2.0));
    FT_ASSERT_EQ(ER_SUCCESS, source.set_radius(9.0));
    FT_ASSERT_EQ(ER_SUCCESS, destination.set_center(-1.0, 1.0, -1.0));
    FT_ASSERT_EQ(ER_SUCCESS, destination.set_radius(1.0));
    destination = std::move(source);
    destination_center_x = destination.get_center_x();
    destination_center_y = destination.get_center_y();
    destination_center_z = destination.get_center_z();
    destination_radius = destination.get_radius();
    source_center_x = source.get_center_x();
    source_center_y = source.get_center_y();
    source_center_z = source.get_center_z();
    source_radius = source.get_radius();
    FT_ASSERT_EQ(7.0, destination_center_x);
    FT_ASSERT_EQ(-3.0, destination_center_y);
    FT_ASSERT_EQ(2.0, destination_center_z);
    FT_ASSERT_EQ(9.0, destination_radius);
    FT_ASSERT_EQ(0.0, source_center_x);
    FT_ASSERT_EQ(0.0, source_center_y);
    FT_ASSERT_EQ(0.0, source_center_z);
    FT_ASSERT_EQ(0.0, source_radius);
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    return (1);
}
