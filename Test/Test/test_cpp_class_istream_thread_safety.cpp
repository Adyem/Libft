#include "../test_internal.hpp"
#include "../../CPP_class/class_istringstream.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <atomic>
#include <chrono>
#include <thread>

#ifndef LIBFT_TEST_BUILD
#endif

class ft_test_counter_istream : public ft_istream
{
    private:
        std::size_t _remaining;
        char _fill_character;

    public:
        ft_test_counter_istream(std::size_t total_count, char fill_character) noexcept
            : ft_istream()
            , _remaining(total_count)
            , _fill_character(fill_character)
        {
            return ;
        }

        std::size_t remaining() const noexcept
        {
            return (this->_remaining);
        }

    protected:
        std::size_t do_read(char *buffer, std::size_t count)
        {
            std::size_t produced;

            if (buffer == ft_nullptr)
            {
                ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
                return (0);
            }
            produced = 0;
            while (produced < count && this->_remaining > 0)
            {
                buffer[produced] = this->_fill_character;
                this->_remaining--;
                produced++;
            }
            if (produced == 0)
            {
                ft_global_error_stack_push(FT_ERR_SUCCESS);
                return (0);
            }
            ft_global_error_stack_push(FT_ERR_SUCCESS);
            return (produced);
        }
};

FT_TEST(test_ft_istream_serializes_concurrent_reads,
        "ft_istream serializes concurrent read operations")
{
    ft_test_counter_istream stream(400, 'q');
    std::atomic<bool> start_flag(false);
    std::atomic<bool> worker_done(false);
    std::atomic<bool> worker_failure(false);
    std::atomic<unsigned int> total_consumed(0);
    std::thread worker;

    worker = std::thread([&stream, &start_flag, &worker_done, &worker_failure, &total_consumed]() {
        while (!start_flag.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        while (true)
        {
            char buffer[2];

            buffer[0] = '\0';
            buffer[1] = '\0';
            stream.read(buffer, 1);
            if (buffer[0] == '\0')
                break ;
            if (buffer[0] != 'q')
                worker_failure.store(true);
            total_consumed.fetch_add(1, std::memory_order_relaxed);
        }
        worker_done.store(true);
    });

    start_flag.store(true);

    while (true)
    {
        char buffer[2];

        buffer[0] = '\0';
        buffer[1] = '\0';
        stream.read(buffer, 1);
        if (buffer[0] == '\0')
            break ;
        FT_ASSERT_EQ('q', buffer[0]);
        total_consumed.fetch_add(1, std::memory_order_relaxed);
    }

    worker.join();

    FT_ASSERT_EQ(false, worker_failure.load());
    FT_ASSERT_EQ(true, worker_done.load());
    FT_ASSERT_EQ(400u, total_consumed.load());
    FT_ASSERT_EQ(true, stream.is_valid());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_global_error_stack_peek_last_error());
    FT_ASSERT_EQ(0u, stream.remaining());
    return (1);
}

FT_TEST(test_ft_istringstream_concurrent_reads,
        "ft_istringstream preserves ordering under concurrent reads")
{
    ft_string pattern("0123456789");
    ft_string repeated;
    unsigned int repeat_index;

    repeat_index = 0;
    while (repeat_index < 40)
    {
        repeated.append(pattern);
        repeat_index++;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_string::last_operation_error());

    ft_istringstream stream(repeated);
    std::atomic<bool> start_flag(false);
    std::atomic<unsigned int> total_count(0);
    std::atomic<bool> mismatch(false);
    std::atomic<unsigned int> digit_counts[10];
    std::thread worker;

    unsigned int digit_index;

    digit_index = 0;
    while (digit_index < 10)
    {
        digit_counts[digit_index].store(0);
        digit_index++;
    }

    worker = std::thread([&stream, &start_flag, &mismatch, &total_count, &digit_counts]() {
        while (!start_flag.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        while (true)
        {
            char buffer[2];

            buffer[0] = '\0';
            buffer[1] = '\0';
            stream.read(buffer, 1);
            if (buffer[0] == '\0')
                break ;
            if (buffer[0] < '0' || buffer[0] > '9')
                mismatch.store(true);
            else
                digit_counts[static_cast<unsigned int>(buffer[0] - '0')].fetch_add(1, std::memory_order_relaxed);
            total_count.fetch_add(1, std::memory_order_relaxed);
        }
    });

    start_flag.store(true);

    while (true)
    {
        char buffer[2];

        buffer[0] = '\0';
        buffer[1] = '\0';
        stream.read(buffer, 1);
        if (buffer[0] == '\0')
            break ;
        if (buffer[0] < '0' || buffer[0] > '9')
            mismatch.store(true);
        else
            digit_counts[static_cast<unsigned int>(buffer[0] - '0')].fetch_add(1, std::memory_order_relaxed);
        total_count.fetch_add(1, std::memory_order_relaxed);
    }

    worker.join();

    FT_ASSERT_EQ(false, mismatch.load());
    FT_ASSERT_EQ(repeated.size(), static_cast<std::size_t>(total_count.load()));

    digit_index = 0;
    while (digit_index < 10)
    {
        FT_ASSERT_EQ(repeated.size() / 10, static_cast<std::size_t>(digit_counts[digit_index].load()));
        digit_index++;
    }
    FT_ASSERT_EQ(true, stream.is_valid());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_global_error_stack_peek_last_error());
    return (1);
}
