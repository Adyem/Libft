#include "../../CPP_class/class_fd_istream.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CPP_class/class_istream.hpp"
#include "../../Template/move.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <chrono>
#include <thread>
#include <unistd.h>

class ft_test_timed_istream : public ft_istream
{
    private:
        char _fill_character;
        bool _slow;

    public:
        ft_test_timed_istream(char fill_character, bool slow) noexcept
            : ft_istream()
            , _fill_character(fill_character)
            , _slow(slow)
        {
            return ;
        }

        void set_slow(bool slow) noexcept
        {
            this->_slow = slow;
            return ;
        }

    protected:
        std::size_t do_read(char *buffer, std::size_t count)
        {
            if (buffer == ft_nullptr || count == 0)
            {
                this->set_error(FT_ERR_INVALID_ARGUMENT);
                return (0);
            }
            if (this->_slow)
                std::this_thread::sleep_for(std::chrono::milliseconds(75));
            buffer[0] = this->_fill_character;
            this->set_error(FT_ERR_SUCCESSS);
            return (1);
        }
};

class ft_test_timed_fd_istream : public ft_fd_istream
{
    private:
        bool _slow;

    public:
        ft_test_timed_fd_istream(int fd, bool slow) noexcept
            : ft_fd_istream(fd)
            , _slow(slow)
        {
            return ;
        }

        void set_slow(bool slow) noexcept
        {
            this->_slow = slow;
            return ;
        }

    protected:
        std::size_t do_read(char *buffer, std::size_t count)
        {
            if (this->_slow)
                std::this_thread::sleep_for(std::chrono::milliseconds(75));
            return (ft_fd_istream::do_read(buffer, count));
        }
};

static long measure_read_duration(ft_istream &stream)
{
    char buffer[2];
    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock::time_point end;

    buffer[0] = '\0';
    buffer[1] = '\0';
    start = std::chrono::steady_clock::now();
    stream.read(buffer, 1);
    end = std::chrono::steady_clock::now();
    return (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

static int create_pipe(int *read_end, int *write_end)
{
    int descriptors[2];
    int result;

    result = pipe(descriptors);
    if (result != 0)
        return (FT_ERR_INVALID_HANDLE);
    *read_end = descriptors[0];
    *write_end = descriptors[1];
    return (FT_ERR_SUCCESSS);
}

static void fill_pipe(int write_end, const char *data)
{
    const char *cursor;

    cursor = data;
    while (*cursor != '\0')
    {
        su_write(write_end, cursor, 1);
        cursor++;
    }
    return ;
}

FT_TEST(test_ft_istream_copy_constructor_mutex_is_fresh,
        "ft_istream copy constructor initializes an independent mutex")
{
    ft_test_timed_istream slow_stream('s', true);
    ft_test_timed_istream copied_stream(slow_stream);
    std::thread worker;
    long copy_duration_ms;

    copied_stream.set_slow(false);
    worker = std::thread([&slow_stream]() {
        char buffer[2];

        buffer[0] = '\0';
        buffer[1] = '\0';
        slow_stream.read(buffer, 1);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    copy_duration_ms = measure_read_duration(copied_stream);
    worker.join();
    FT_ASSERT(copy_duration_ms < 60);
    FT_ASSERT_EQ(false, copied_stream.bad());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, copied_stream.get_error());
    return (1);
}

FT_TEST(test_ft_istream_move_constructor_mutex_is_fresh,
        "ft_istream move constructor initializes an independent mutex")
{
    ft_test_timed_istream slow_stream('m', true);
    ft_test_timed_istream moved_stream(ft_move(slow_stream));
    std::thread worker;
    long copy_duration_ms;

    moved_stream.set_slow(false);
    worker = std::thread([&slow_stream]() {
        char buffer[2];

        buffer[0] = '\0';
        buffer[1] = '\0';
        slow_stream.read(buffer, 1);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    copy_duration_ms = measure_read_duration(moved_stream);
    worker.join();
    FT_ASSERT(copy_duration_ms < 60);
    FT_ASSERT_EQ(false, moved_stream.bad());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, moved_stream.get_error());
    FT_ASSERT_EQ(false, slow_stream.bad());
    return (1);
}

FT_TEST(test_ft_istream_copy_assignment_mutex_is_fresh,
        "ft_istream copy assignment initializes an independent mutex")
{
    ft_test_timed_istream target_stream('t', true);
    ft_test_timed_istream source_stream('u', false);
    std::thread worker;
    long copy_duration_ms;

    target_stream = source_stream;
    source_stream.set_slow(true);
    target_stream.set_slow(false);
    worker = std::thread([&source_stream]() {
        char buffer[2];

        buffer[0] = '\0';
        buffer[1] = '\0';
        source_stream.read(buffer, 1);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    copy_duration_ms = measure_read_duration(target_stream);
    worker.join();
    FT_ASSERT(copy_duration_ms < 60);
    FT_ASSERT_EQ(false, target_stream.bad());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, target_stream.get_error());
    return (1);
}

FT_TEST(test_ft_istream_move_assignment_mutex_is_fresh,
        "ft_istream move assignment initializes an independent mutex")
{
    ft_test_timed_istream target_stream('x', true);
    ft_test_timed_istream source_stream('y', true);
    std::thread worker;
    long copy_duration_ms;

    target_stream = ft_move(source_stream);
    target_stream.set_slow(false);
    source_stream.set_slow(true);
    worker = std::thread([&source_stream]() {
        char buffer[2];

        buffer[0] = '\0';
        buffer[1] = '\0';
        source_stream.read(buffer, 1);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    copy_duration_ms = measure_read_duration(target_stream);
    worker.join();
    FT_ASSERT(copy_duration_ms < 60);
    FT_ASSERT_EQ(false, target_stream.bad());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, target_stream.get_error());
    FT_ASSERT_EQ(false, source_stream.bad());
    return (1);
}

FT_TEST(test_ft_fd_istream_copy_constructor_mutex_is_fresh,
        "ft_fd_istream copy constructor initializes an independent mutex")
{
    int read_end;
    int write_end;
    int pipe_error;
    std::thread worker;
    long copy_duration_ms;

    pipe_error = create_pipe(&read_end, &write_end);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, pipe_error);
    ft_test_timed_fd_istream slow_stream(read_end, true);
    ft_test_timed_fd_istream copied_stream(slow_stream);
    copied_stream.set_slow(false);
    fill_pipe(write_end, "ab");
    worker = std::thread([&slow_stream]() {
        char buffer[2];

        buffer[0] = '\0';
        buffer[1] = '\0';
        slow_stream.read(buffer, 1);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    copy_duration_ms = measure_read_duration(copied_stream);
    worker.join();
    close(write_end);
    close(read_end);
    FT_ASSERT(copy_duration_ms < 60);
    FT_ASSERT_EQ(false, copied_stream.bad());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, copied_stream.get_error());
    return (1);
}

FT_TEST(test_ft_fd_istream_move_constructor_mutex_is_fresh,
        "ft_fd_istream move constructor initializes an independent mutex")
{
    int read_end;
    int write_end;
    int pipe_error;
    std::thread worker;
    long copy_duration_ms;

    pipe_error = create_pipe(&read_end, &write_end);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, pipe_error);
    ft_test_timed_fd_istream slow_stream(read_end, true);
    ft_test_timed_fd_istream moved_stream(ft_move(slow_stream));
    slow_stream.set_fd(read_end);
    moved_stream.set_fd(read_end);
    moved_stream.set_slow(false);
    fill_pipe(write_end, "cd");
    worker = std::thread([&slow_stream]() {
        char buffer[2];

        buffer[0] = '\0';
        buffer[1] = '\0';
        slow_stream.read(buffer, 1);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    copy_duration_ms = measure_read_duration(moved_stream);
    worker.join();
    close(write_end);
    close(read_end);
    FT_ASSERT(copy_duration_ms < 60);
    FT_ASSERT_EQ(false, moved_stream.bad());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, moved_stream.get_error());
    FT_ASSERT_EQ(false, slow_stream.bad());
    return (1);
}

FT_TEST(test_ft_fd_istream_copy_assignment_mutex_is_fresh,
        "ft_fd_istream copy assignment initializes an independent mutex")
{
    int read_end;
    int write_end;
    int pipe_error;
    ft_test_timed_fd_istream target_stream(0, true);
    ft_test_timed_fd_istream source_stream(0, false);
    std::thread worker;
    long copy_duration_ms;

    pipe_error = create_pipe(&read_end, &write_end);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, pipe_error);
    target_stream.set_fd(read_end);
    source_stream.set_fd(read_end);
    target_stream = source_stream;
    target_stream.set_slow(false);
    source_stream.set_slow(true);
    fill_pipe(write_end, "ef");
    worker = std::thread([&source_stream]() {
        char buffer[2];

        buffer[0] = '\0';
        buffer[1] = '\0';
        source_stream.read(buffer, 1);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    copy_duration_ms = measure_read_duration(target_stream);
    worker.join();
    close(write_end);
    close(read_end);
    FT_ASSERT(copy_duration_ms < 60);
    FT_ASSERT_EQ(false, target_stream.bad());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, target_stream.get_error());
    return (1);
}

FT_TEST(test_ft_fd_istream_move_assignment_mutex_is_fresh,
        "ft_fd_istream move assignment initializes an independent mutex")
{
    int read_end;
    int write_end;
    int pipe_error;
    ft_test_timed_fd_istream target_stream(0, true);
    ft_test_timed_fd_istream source_stream(0, true);
    std::thread worker;
    long copy_duration_ms;

    pipe_error = create_pipe(&read_end, &write_end);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, pipe_error);
    target_stream.set_fd(read_end);
    source_stream.set_fd(read_end);
    target_stream = ft_move(source_stream);
    target_stream.set_slow(false);
    source_stream.set_fd(read_end);
    source_stream.set_slow(true);
    fill_pipe(write_end, "gh");
    worker = std::thread([&source_stream]() {
        char buffer[2];

        buffer[0] = '\0';
        buffer[1] = '\0';
        source_stream.read(buffer, 1);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    copy_duration_ms = measure_read_duration(target_stream);
    worker.join();
    close(write_end);
    close(read_end);
    FT_ASSERT(copy_duration_ms < 60);
    FT_ASSERT_EQ(false, target_stream.bad());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, target_stream.get_error());
    FT_ASSERT_EQ(false, source_stream.bad());
    return (1);
}
