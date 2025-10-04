#include "../../Template/shared_ptr.hpp"
#include "../../Template/atomic.hpp"
#include "../../PThread/thread.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <new>

FT_TEST(test_template_shared_ptr_thread_safe_reference_counting, "ft_sharedptr reference count updates are synchronized")
{
    int *raw_pointer;

    raw_pointer = new (std::nothrow) int(0);
    FT_ASSERT(raw_pointer != ft_nullptr);
    ft_sharedptr<int> shared_pointer(raw_pointer);
    ft_atomic<int> copy_count(0);

    auto copier = [&shared_pointer, &copy_count]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 120)
        {
            ft_sharedptr<int> local_copy(shared_pointer);
            if (!local_copy.hasError())
            {
                copy_count.fetch_add(1);
            }
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread first_thread(copier);
    ft_thread second_thread(copier);

    first_thread.join();
    second_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, first_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, second_thread.get_error());
    FT_ASSERT(copy_count.load() >= 240);
    FT_ASSERT_EQ(1, shared_pointer.use_count());
    FT_ASSERT_EQ(ER_SUCCESS, shared_pointer.get_error());
    return (1);
}

FT_TEST(test_template_shared_ptr_thread_safe_reset_during_copy, "ft_sharedptr reset synchronizes with concurrent copies")
{
    int *initial_pointer;
    int *replacement_pointer;

    initial_pointer = new (std::nothrow) int(5);
    replacement_pointer = new (std::nothrow) int(9);
    FT_ASSERT(initial_pointer != ft_nullptr && replacement_pointer != ft_nullptr);
    ft_sharedptr<int> shared_pointer(initial_pointer);
    ft_atomic<int> copy_successes(0);

    auto copier = [&shared_pointer, &copy_successes]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 80)
        {
            ft_sharedptr<int> copy_instance(shared_pointer);
            if (!copy_instance.hasError())
            {
                copy_successes.fetch_add(1);
            }
            iteration = iteration + 1;
        }
        return ;
    };
    auto resetter = [&shared_pointer, &replacement_pointer]()
    {
        pt_thread_sleep(40);
        shared_pointer.reset(replacement_pointer, 1, false);
        replacement_pointer = ft_nullptr;
        return ;
    };

    ft_thread copy_thread(copier);
    ft_thread reset_thread(resetter);

    copy_thread.join();
    reset_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, copy_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, reset_thread.get_error());
    FT_ASSERT(copy_successes.load() >= 80);
    FT_ASSERT_EQ(ER_SUCCESS, shared_pointer.get_error());
    FT_ASSERT_EQ(9, *shared_pointer);
    return (1);
}

FT_TEST(test_template_shared_ptr_thread_safe_swap, "ft_sharedptr swap avoids deadlocks under contention")
{
    int *first_pointer;
    int *second_pointer;

    first_pointer = new (std::nothrow) int(1);
    second_pointer = new (std::nothrow) int(2);
    FT_ASSERT(first_pointer != ft_nullptr && second_pointer != ft_nullptr);
    ft_sharedptr<int> first_shared(first_pointer);
    ft_sharedptr<int> second_shared(second_pointer);

    auto first_swapper = [&first_shared, &second_shared]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 100)
        {
            first_shared.swap(second_shared);
            iteration = iteration + 1;
        }
        return ;
    };
    auto second_swapper = [&first_shared, &second_shared]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 100)
        {
            second_shared.swap(first_shared);
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread swapper_one(first_swapper);
    ft_thread swapper_two(second_swapper);

    swapper_one.join();
    swapper_two.join();

    FT_ASSERT_EQ(ER_SUCCESS, swapper_one.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, swapper_two.get_error());
    int combined_value;

    combined_value = *first_shared + *second_shared;
    FT_ASSERT_EQ(3, combined_value);
    FT_ASSERT_EQ(ER_SUCCESS, first_shared.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, second_shared.get_error());
    return (1);
}
