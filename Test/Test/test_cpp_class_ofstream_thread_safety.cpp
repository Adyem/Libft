#include "../../CPP_class/class_ofstream.hpp"
#include "../../CPP_class/class_file.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <atomic>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <fcntl.h>

FT_TEST(test_ft_ofstream_concurrent_writes_are_serialized,
        "ft_ofstream serializes concurrent write operations")
{
    const char *filename;
    ft_ofstream stream;
    std::atomic<bool> start_flag;
    std::atomic<bool> worker_done;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    unsigned int iteration;
    const char *main_chunk;
    ssize_t write_result;

    filename = "test_ft_ofstream_concurrent_writes_are_serialized.txt";
    start_flag.store(false);
    worker_done.store(false);
    worker_failed.store(false);

    FT_ASSERT_EQ(0, stream.open(filename));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, stream.get_error());

    worker_thread = std::thread([&stream, &start_flag, &worker_done, &worker_failed]() {
        unsigned int iteration_local;
        const char *worker_chunk;
        ssize_t write_result_local;

        iteration_local = 0;
        worker_chunk = "x";
        while (!start_flag.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        while (iteration_local < 200)
        {
            write_result_local = stream.write(worker_chunk);
            if (write_result_local < 0)
            {
                worker_failed.store(true);
                break ;
            }
            iteration_local++;
        }
        worker_done.store(true);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    start_flag.store(true);

    iteration = 0;
    main_chunk = "y";
    while (iteration < 200)
    {
        write_result = stream.write(main_chunk);
        if (write_result < 0)
            break ;
        iteration++;
    }

    while (!worker_done.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    worker_thread.join();

    FT_ASSERT_EQ(false, worker_failed.load());
    FT_ASSERT_EQ(200u, iteration);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, stream.get_error());

    stream.close();
    FT_ASSERT_EQ(FT_ER_SUCCESSS, stream.get_error());

    ft_file reader(filename, O_RDONLY);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, reader.get_error());

    char file_contents[401];
    ssize_t read_result;
    ssize_t total_read;
    unsigned int index;
    unsigned int count_x;
    unsigned int count_y;

    total_read = 0;
    while (total_read < 400)
    {
        read_result = reader.read(file_contents + total_read, 400 - total_read);
        if (read_result <= 0)
            break ;
        total_read += read_result;
    }
    FT_ASSERT_EQ(400, static_cast<int>(total_read));
    file_contents[total_read] = '\0';

    index = 0;
    count_x = 0;
    count_y = 0;
    while (index < static_cast<unsigned int>(total_read))
    {
        if (file_contents[index] == 'x')
            count_x++;
        else if (file_contents[index] == 'y')
            count_y++;
        index++;
    }

    FT_ASSERT_EQ(200u, count_x);
    FT_ASSERT_EQ(200u, count_y);

    FT_ASSERT_EQ(0, ::unlink(filename));
    return (1);
}
