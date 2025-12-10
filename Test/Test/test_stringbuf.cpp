#include "../../CPP_class/class_stringbuf.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include <atomic>
#include <chrono>
#include <thread>

FT_TEST(test_ft_stringbuf_read_basic, "ft_stringbuf::read copies data sequentially")
{
    ft_string source("hello");
    ft_stringbuf buffer(source);
    char storage[8];
    std::size_t bytes_read;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    bytes_read = buffer.read(storage, 5);
    storage[bytes_read] = '\0';
    FT_ASSERT_EQ(static_cast<std::size_t>(5), bytes_read);
    FT_ASSERT_EQ(0, ft_strcmp(storage, "hello"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, buffer.get_error());
    FT_ASSERT_EQ(false, buffer.is_bad());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_stringbuf_read_null_buffer_sets_error, "ft_stringbuf::read reports errors for null buffers")
{
    ft_string source("data");
    ft_stringbuf buffer(source);
    std::size_t bytes_read;

    ft_errno = FT_ER_SUCCESSS;
    bytes_read = buffer.read(ft_nullptr, 3);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), bytes_read);
    FT_ASSERT_EQ(true, buffer.is_bad());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, buffer.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_ft_stringbuf_str_returns_remaining, "ft_stringbuf::str exposes unread portion and clears errors")
{
    ft_string source("abcdef");
    ft_stringbuf buffer(source);
    char storage[4];
    std::size_t bytes_read;
    ft_string remaining;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    bytes_read = buffer.read(storage, 3);
    storage[bytes_read] = '\0';
    FT_ASSERT_EQ(static_cast<std::size_t>(3), bytes_read);
    FT_ASSERT_EQ(0, ft_strcmp(storage, "abc"));

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    remaining = buffer.str();
    FT_ASSERT_EQ(0, ft_strcmp(remaining.c_str(), "def"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, buffer.get_error());
    FT_ASSERT_EQ(false, buffer.is_bad());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_stringbuf_read_past_end_returns_zero, "ft_stringbuf::read returns zero after reaching the end")
{
    ft_string source("xy");
    ft_stringbuf buffer(source);
    char storage[4];
    std::size_t bytes_read;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    bytes_read = buffer.read(storage, 2);
    storage[bytes_read] = '\0';
    FT_ASSERT_EQ(static_cast<std::size_t>(2), bytes_read);
    FT_ASSERT_EQ(0, ft_strcmp(storage, "xy"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, buffer.get_error());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    bytes_read = buffer.read(storage, 2);
    FT_ASSERT_EQ(static_cast<std::size_t>(0), bytes_read);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, buffer.get_error());
    FT_ASSERT_EQ(false, buffer.is_bad());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_stringbuf_concurrent_reads_are_serialized,
        "ft_stringbuf serializes concurrent read operations")
{
    const char *seed_data;
    std::atomic<bool> start_flag;
    std::atomic<bool> worker_done;
    ft_string worker_output;
    ft_string main_output;
    std::thread worker_thread;

    seed_data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    ft_string source(seed_data);
    ft_stringbuf buffer(source);
    start_flag.store(false);
    worker_done.store(false);
    worker_thread = std::thread([&buffer, &start_flag, &worker_done, &worker_output]() {
        char single_character[2];
        std::size_t bytes_read;

        while (!start_flag.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        while (true)
        {
            bytes_read = buffer.read(single_character, 1);
            if (bytes_read == 0)
                break ;
            single_character[1] = '\0';
            worker_output.append(single_character, 1);
        }
        worker_done.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    start_flag.store(true);

    char single_character[2];
    std::size_t bytes_read;

    while (true)
    {
        bytes_read = buffer.read(single_character, 1);
        if (bytes_read == 0)
            break ;
        single_character[1] = '\0';
        main_output.append(single_character, 1);
    }

    while (!worker_done.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    worker_thread.join();

    FT_ASSERT_EQ(source.size(), worker_output.size() + main_output.size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, buffer.get_error());

    ft_string remaining;

    remaining = buffer.str();
    FT_ASSERT_EQ(0u, remaining.size());

    bool seen[128];
    unsigned int seen_index;

    seen_index = 0;
    while (seen_index < 128)
    {
        seen[seen_index] = false;
        seen_index++;
    }

    const char *worker_chars;
    unsigned int worker_index;

    worker_chars = worker_output.c_str();
    worker_index = 0;
    while (worker_index < worker_output.size())
    {
        unsigned char value;

        value = static_cast<unsigned char>(worker_chars[worker_index]);
        FT_ASSERT_EQ(false, seen[value]);
        seen[value] = true;
        worker_index++;
    }

    const char *main_chars;
    unsigned int main_index;

    main_chars = main_output.c_str();
    main_index = 0;
    while (main_index < main_output.size())
    {
        unsigned char value;

        value = static_cast<unsigned char>(main_chars[main_index]);
        FT_ASSERT_EQ(false, seen[value]);
        seen[value] = true;
        main_index++;
    }

    const char *source_chars;
    unsigned int source_index;

    source_chars = source.c_str();
    source_index = 0;
    while (source_index < source.size())
    {
        unsigned char value;

        value = static_cast<unsigned char>(source_chars[source_index]);
        FT_ASSERT_EQ(true, seen[value]);
        source_index++;
    }

    return (1);
}
