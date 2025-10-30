#include "../../CPP_class/class_string_class.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

FT_TEST(test_ft_string_append_preserves_errno,
        "ft_string append restores ft_errno after completing")
{
    ft_string string_value;
    int       saved_errno;

    saved_errno = FT_ERR_INVALID_ARGUMENT;
    ft_errno = saved_errno;
    string_value.append('x');
    FT_ASSERT_EQ(saved_errno, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, string_value.get_error());
    FT_ASSERT_EQ(1u, string_value.size());
    return (1);
}

FT_TEST(test_ft_string_concurrent_appends_are_serialized,
        "ft_string serializes concurrent append operations")
{
    ft_string            shared_string;
    std::atomic<bool>    start_flag;
    std::atomic<bool>    worker_done;
    std::thread          worker;

    start_flag.store(false);
    worker_done.store(false);
    worker = std::thread([&shared_string, &start_flag, &worker_done]() {
        while (!start_flag.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        unsigned int iteration;

        iteration = 0;
        while (iteration < 500)
        {
            shared_string.append('a');
            iteration++;
        }
        worker_done.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    start_flag.store(true);

    unsigned int iteration;

    iteration = 0;
    while (iteration < 500)
    {
        shared_string.append('b');
        iteration++;
    }

    while (!worker_done.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    worker.join();

    FT_ASSERT_EQ(ER_SUCCESS, shared_string.get_error());
    FT_ASSERT_EQ(1000u, shared_string.size());

    const char *contents;
    unsigned int index;
    unsigned int count_a;
    unsigned int count_b;

    contents = shared_string.c_str();
    index = 0;
    count_a = 0;
    count_b = 0;
    while (index < shared_string.size())
    {
        if (contents[index] == 'a')
            count_a++;
        else if (contents[index] == 'b')
            count_b++;
        index++;
    }
    FT_ASSERT_EQ(500u, count_a);
    FT_ASSERT_EQ(500u, count_b);
    return (1);
}

FT_TEST(test_ft_string_append_of_error_string_propagates_code,
        "ft_string append propagates source error state")
{
    ft_string healthy_string("ok");
    ft_string failing_string(FT_ERR_NO_MEMORY);

    healthy_string.append(failing_string);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, healthy_string.get_error());
    return (1);
}
