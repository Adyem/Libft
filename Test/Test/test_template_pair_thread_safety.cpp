#include "../../Template/pair.hpp"
#include <atomic>
#include "../../PThread/thread.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_template_pair_thread_safe_concurrent_mutation, "Pair guards concurrent key and value updates")
{
    Pair<int, int> pair_instance(0, 0);

    auto update_key = [&pair_instance]()
    {
        int index;
        index = 0;
        while (index < 200)
        {
            pair_instance.set_key(index);
            index = index + 1;
        }
        return ;
    };
    auto update_value = [&pair_instance]()
    {
        int index;
        index = 0;
        while (index < 200)
        {
            pair_instance.set_value(index);
            index = index + 1;
        }
        return ;
    };

    ft_thread key_thread(update_key);
    ft_thread value_thread(update_value);

    key_thread.join();
    value_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, key_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, value_thread.get_error());
    FT_ASSERT_EQ(199, pair_instance.get_key());
    FT_ASSERT_EQ(199, pair_instance.get_value());
    FT_ASSERT_EQ(ER_SUCCESS, pair_instance.get_error());
    return (1);
}

FT_TEST(test_template_pair_thread_safe_cross_assignment, "Pair avoids deadlock when instances assign each other")
{
    Pair<int, int> first_pair(1, 10);
    Pair<int, int> second_pair(2, 20);

    auto assign_first = [&first_pair, &second_pair]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 50)
        {
            first_pair = second_pair;
            iteration = iteration + 1;
        }
        return ;
    };
    auto assign_second = [&first_pair, &second_pair]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 50)
        {
            second_pair = first_pair;
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread first_thread(assign_first);
    ft_thread second_thread(assign_second);

    first_thread.join();
    second_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, first_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, second_thread.get_error());
    int final_key_first = first_pair.get_key();
    int final_key_second = second_pair.get_key();
    FT_ASSERT((final_key_first == 1 && final_key_second == 1) || (final_key_first == 2 && final_key_second == 2));
    FT_ASSERT_EQ(ER_SUCCESS, first_pair.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, second_pair.get_error());
    return (1);
}

FT_TEST(test_template_pair_thread_safe_get_during_updates, "Pair getters remain consistent during concurrent writes")
{
    Pair<int, int> monitored_pair(5, 50);
    std::atomic<int> successful_reads(0);

    auto reader = [&monitored_pair, &successful_reads]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 100)
        {
            monitored_pair.get_key();
            if (monitored_pair.get_error() == ER_SUCCESS)
            {
                successful_reads.fetch_add(1);
            }
            iteration = iteration + 1;
        }
        return ;
    };
    auto writer = [&monitored_pair]()
    {
        int value;
        value = 0;
        while (value < 100)
        {
            monitored_pair.set_value(value);
            value = value + 1;
        }
        return ;
    };

    ft_thread reader_thread(reader);
    ft_thread writer_thread(writer);

    reader_thread.join();
    writer_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, reader_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, writer_thread.get_error());
    FT_ASSERT_EQ(100, successful_reads.load());
    FT_ASSERT_EQ(ER_SUCCESS, monitored_pair.get_error());
    return (1);
}
