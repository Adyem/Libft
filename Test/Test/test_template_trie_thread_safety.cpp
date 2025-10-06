#include "../../Template/trie.hpp"
#include <atomic>
#include "../../PThread/thread.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_template_trie_thread_safe_concurrent_insert, "ft_trie handles concurrent insert operations")
{
    ft_trie<int> trie_instance;
    int value_alpha = 10;
    int value_beta = 20;
    std::atomic<int> insert_success(0);

    auto insert_alpha = [&trie_instance, &value_alpha, &insert_success]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 60)
        {
            if (trie_instance.insert("alpha", &value_alpha) == 0)
            {
                insert_success.fetch_add(1);
            }
            iteration = iteration + 1;
        }
        return ;
    };
    auto insert_beta = [&trie_instance, &value_beta, &insert_success]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 60)
        {
            if (trie_instance.insert("beta", &value_beta) == 0)
            {
                insert_success.fetch_add(1);
            }
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread alpha_thread(insert_alpha);
    ft_thread beta_thread(insert_beta);

    alpha_thread.join();
    beta_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, alpha_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, beta_thread.get_error());
    FT_ASSERT(insert_success.load() >= 120);
    const void *alpha_node;
    const void *beta_node;

    alpha_node = trie_instance.search("alpha");
    beta_node = trie_instance.search("beta");
    FT_ASSERT(alpha_node != ft_nullptr && beta_node != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, trie_instance.get_error());
    return (1);
}

FT_TEST(test_template_trie_thread_safe_insert_and_search, "ft_trie search sees inserted values concurrently")
{
    ft_trie<int> trie_instance;
    int value_gamma = 30;
    std::atomic<int> found_count(0);

    auto inserter = [&trie_instance, &value_gamma]()
    {
        pt_thread_sleep(20);
        trie_instance.insert("gamma", &value_gamma);
        return ;
    };
    auto searcher = [&trie_instance, &found_count]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 120)
        {
            const void *node;

            node = trie_instance.search("gamma");
            if (node != ft_nullptr)
            {
                found_count.fetch_add(1);
            }
            else
            {
                pt_thread_yield();
            }
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread insert_thread(inserter);
    ft_thread search_thread(searcher);

    insert_thread.join();
    search_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, insert_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, search_thread.get_error());
    FT_ASSERT(found_count.load() > 0);
    FT_ASSERT_EQ(ER_SUCCESS, trie_instance.get_error());
    return (1);
}

FT_TEST(test_template_trie_thread_safe_repeated_searches, "ft_trie search remains stable under contention")
{
    ft_trie<int> trie_instance;
    int value_delta = 40;
    trie_instance.insert("delta", &value_delta);
    FT_ASSERT_EQ(ER_SUCCESS, trie_instance.get_error());
    std::atomic<int> successful_searches(0);

    auto search_worker = [&trie_instance, &successful_searches]()
    {
        int iteration;
        iteration = 0;
        while (iteration < 150)
        {
            const void *node;

            node = trie_instance.search("delta");
            if (node != ft_nullptr)
            {
                successful_searches.fetch_add(1);
            }
            iteration = iteration + 1;
        }
        return ;
    };

    ft_thread first_thread(search_worker);
    ft_thread second_thread(search_worker);

    first_thread.join();
    second_thread.join();

    FT_ASSERT_EQ(ER_SUCCESS, first_thread.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, second_thread.get_error());
    FT_ASSERT(successful_searches.load() >= 300);
    FT_ASSERT_EQ(ER_SUCCESS, trie_instance.get_error());
    return (1);
}
