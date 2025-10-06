#include "../../Template/string_view.hpp"
#include <atomic>
#include "../../PThread/thread.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_template_string_view_thread_safe_copy_assignment, "ft_string_view copy assignment avoids deadlock")
{
    const char sample_text[] = "concurrency";
    ft_string_view<char> original_view(sample_text);
    ft_string_view<char> secondary_view(sample_text);
    std::atomic<int> completed_assignments(0);

    auto assigner = [&original_view, &secondary_view, &completed_assignments]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 120)
        {
            secondary_view = original_view;
            if (secondary_view.get_error() == ER_SUCCESS)
            {
                completed_assignments.fetch_add(1);
            }
            iteration = iteration + 1;
        }
        return ;
    };
    auto copier = [&original_view, &completed_assignments]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 120)
        {
            ft_string_view<char> copy_view(original_view);
            if (copy_view.get_error() == ER_SUCCESS)
            {
                completed_assignments.fetch_add(1);
            }
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread assign_thread(assigner);
    ft_thread copy_thread(copier);

    assign_thread.join();
    copy_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, assign_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, copy_thread.get_error());
    FT_ASSERT(completed_assignments.load() >= 240);
    FT_ASSERT_EQ(ER_SUCCESS, original_view.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, secondary_view.get_error());
    return (1);
}

FT_TEST(test_template_string_view_thread_safe_concurrent_access, "ft_string_view accessors synchronize with assignments")
{
    const char initial_text[] = "thread";
    const char alternate_text[] = "safety";
    ft_string_view<char> main_view(initial_text);
    ft_string_view<char> other_view(alternate_text);
    std::atomic<int> size_sum(0);

    auto accessor = [&main_view, &size_sum]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 150)
        {
            size_t current_size;

            current_size = main_view.size();
            if (main_view.get_error() == ER_SUCCESS)
            {
                size_sum.fetch_add(static_cast<int>(current_size));
            }
            iteration = iteration + 1;
        }
        return ;
    };
    auto assigner = [&main_view, &other_view]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 150)
        {
            if ((iteration & 1) == 0)
            {
                main_view = other_view;
            }
            else
            {
                main_view = ft_string_view<char>("thread");
            }
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread access_thread(accessor);
    ft_thread assign_thread(assigner);

    access_thread.join();
    assign_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, access_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, assign_thread.get_error());
    FT_ASSERT(size_sum.load() > 0);
    FT_ASSERT_EQ(ER_SUCCESS, main_view.get_error());
    return (1);
}

FT_TEST(test_template_string_view_thread_safe_substring_operations, "ft_string_view substr and compare remain consistent concurrently")
{
    const char base_text[] = "synchronized";
    ft_string_view<char> base_view(base_text);
    std::atomic<int> successful_operations(0);

    auto substring_worker = [&base_view, &successful_operations]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 90)
        {
            ft_string_view<char> sub_view = base_view.substr(0, 5);
            if (base_view.get_error() == ER_SUCCESS && sub_view.get_error() == ER_SUCCESS)
            {
                successful_operations.fetch_add(1);
            }
            iteration = iteration + 1;
        }
        return ;
    };
    auto comparer = [&base_view, &successful_operations]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 90)
        {
            ft_string_view<char> other_view("sync");
            int comparison_result;

            comparison_result = base_view.compare(other_view);
            (void)comparison_result;
            if (base_view.get_error() == ER_SUCCESS)
            {
                successful_operations.fetch_add(1);
            }
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread substring_thread(substring_worker);
    ft_thread compare_thread(comparer);

    substring_thread.join();
    compare_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, substring_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, compare_thread.get_error());
    FT_ASSERT(successful_operations.load() >= 180);
    FT_ASSERT_EQ(ER_SUCCESS, base_view.get_error());
    return (1);
}
