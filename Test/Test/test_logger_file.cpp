#include "../../Logger/logger.hpp"
#include "../../Logger/logger_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/system_utils.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#include <sys/stat.h>
#include <utime.h>
#include <ctime>

static int  g_file_sink_hook_calls = 0;

static ssize_t    logger_partial_write_hook(int file_descriptor, const void *buffer, size_t count)
{
    size_t  chunk_size;
    ssize_t write_result;

    g_file_sink_hook_calls += 1;
    chunk_size = 4;
    if (count < chunk_size)
        chunk_size = count;
    write_result = write(file_descriptor, buffer, chunk_size);
    if (write_result < 0)
        return (write_result);
    return (write_result);
}

FT_TEST(test_logger_file_sink_prepare_thread_safety_initializes_mutex,
        "file_sink_prepare_thread_safety allocates a mutex guard")
{
    char        template_path[] = "/tmp/libft_logger_file_sink_mutex_XXXXXX";
    int         temp_fd;
    s_file_sink sink;
    bool        lock_acquired;

    temp_fd = mkstemp(template_path);
    FT_ASSERT(temp_fd >= 0);
    sink.fd = temp_fd;
    sink.path = ft_string(template_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, sink.path.get_error());
    sink.max_size = 0;
    sink.retention_count = 1;
    sink.max_age_seconds = 0;
    FT_ASSERT_EQ(0, file_sink_prepare_thread_safety(&sink));
    lock_acquired = false;
    FT_ASSERT_EQ(0, file_sink_lock(&sink, &lock_acquired));
    FT_ASSERT(lock_acquired == true);
    file_sink_unlock(&sink, lock_acquired);
    file_sink_teardown_thread_safety(&sink);
    close(temp_fd);
    unlink(template_path);
    return (1);
}

FT_TEST(test_logger_file_sink_lock_blocks_until_release,
        "file_sink_lock waits for active writers to release the mutex")
{
    char                     template_path[] = "/tmp/libft_logger_file_sink_lock_XXXXXX";
    int                      temp_fd;
    s_file_sink              sink;
    bool                     main_lock_acquired;
    std::atomic<bool>        ready;
    std::atomic<bool>        worker_failed;
    std::atomic<long long>   wait_duration_ms;
    std::thread              worker;

    temp_fd = mkstemp(template_path);
    FT_ASSERT(temp_fd >= 0);
    sink.fd = temp_fd;
    sink.path = ft_string(template_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, sink.path.get_error());
    sink.max_size = 0;
    sink.retention_count = 1;
    sink.max_age_seconds = 0;
    FT_ASSERT_EQ(0, file_sink_prepare_thread_safety(&sink));
    main_lock_acquired = false;
    FT_ASSERT_EQ(0, file_sink_lock(&sink, &main_lock_acquired));
    FT_ASSERT(main_lock_acquired == true);
    ready.store(false);
    worker_failed.store(false);
    wait_duration_ms.store(0);
    worker = std::thread([&sink, &ready, &worker_failed, &wait_duration_ms]() {
        bool                                       worker_lock_acquired;
        std::chrono::steady_clock::time_point      start_time;
        std::chrono::steady_clock::time_point      end_time;

        worker_lock_acquired = false;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (file_sink_lock(&sink, &worker_lock_acquired) != 0)
        {
            worker_failed.store(true);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        if (!worker_lock_acquired)
            worker_failed.store(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        file_sink_unlock(&sink, worker_lock_acquired);
    });
    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    file_sink_unlock(&sink, main_lock_acquired);
    worker.join();
    FT_ASSERT(worker_failed.load() == false);
    FT_ASSERT(wait_duration_ms.load() >= 40);
    file_sink_teardown_thread_safety(&sink);
    close(temp_fd);
    unlink(template_path);
    return (1);
}

FT_TEST(test_logger_file_sink_uses_system_utils_write, "file sink routes writes through su_write")
{
    char    template_path[] = "/tmp/libft_logger_file_sink_XXXXXX";
    int     temp_fd;
    char    read_buffer[512];
    ssize_t read_count;
    int     log_fd;

    temp_fd = mkstemp(template_path);
    FT_ASSERT(temp_fd >= 0);
    close(temp_fd);
    ft_log_close();
    FT_ASSERT_EQ(0, ft_log_set_file(template_path, 4096));
    g_file_sink_hook_calls = 0;
    su_set_write_syscall_hook(logger_partial_write_hook);
    ft_log_info("file sink test message");
    su_reset_write_syscall_hook();
    FT_ASSERT(g_file_sink_hook_calls > 1);
    ft_log_close();
    log_fd = open(template_path, O_RDONLY);
    FT_ASSERT(log_fd >= 0);
    read_count = read(log_fd, read_buffer, sizeof(read_buffer) - 1);
    FT_ASSERT(read_count > 0);
    read_buffer[read_count] = '\0';
    FT_ASSERT(ft_strstr(read_buffer, "file sink test message") != ft_nullptr);
    close(log_fd);
    unlink(template_path);
    return (1);
}

FT_TEST(test_logger_set_file_missing_directory, "ft_log_set_file returns errno for missing directory")
{
    const char *directory_path;
    const char *file_path;
    int result;

    directory_path = "/tmp/libft_logger_missing_dir";
    file_path = "/tmp/libft_logger_missing_dir/log.txt";
    (void)rmdir(directory_path);
    errno = 0;
    ft_errno = FT_ERR_SUCCESSS;
    result = ft_log_set_file(file_path, 1024);
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_logger_rotate_fstat_failure_sets_errno, "ft_log_rotate reports fstat failure")
{
    s_file_sink sink;

    sink.fd = -1;
    sink.path = ft_string("/tmp/libft_logger_invalid_fd");
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, sink.path.get_error());
    sink.max_size = 1;
    errno = 0;
    ft_errno = FT_ERR_SUCCESSS;
    ft_log_rotate(&sink);
    FT_ASSERT_EQ(FT_ERR_INVALID_HANDLE, ft_errno);
    return (1);
}

FT_TEST(test_logger_rotate_success_clears_errno, "ft_log_rotate clears errno after successful rotation")
{
    char        template_path[] = "/tmp/libft_logger_rotate_XXXXXX";
    int         temp_fd;
    ssize_t     write_result;
    s_file_sink sink;
    ft_string   rotated_path;

    temp_fd = mkstemp(template_path);
    FT_ASSERT(temp_fd >= 0);
    write_result = write(temp_fd, "rotation-test", 13);
    FT_ASSERT_EQ(13, write_result);
    sink.fd = temp_fd;
    sink.path = ft_string(template_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, sink.path.get_error());
    sink.max_size = 4;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    errno = 0;
    ft_log_rotate(&sink);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT(sink.fd >= 0);
    rotated_path = sink.path + ".1";
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rotated_path.get_error());
    FT_ASSERT_EQ(0, access(rotated_path.c_str(), F_OK));
    close(sink.fd);
    unlink(template_path);
    unlink(rotated_path.c_str());
    return (1);
}

FT_TEST(test_logger_rotate_rename_failure_reopens_file, "ft_log_rotate reopens original file when rename fails")
{
    char        directory_template[] = "/tmp/libft_logger_rotate_fail_XXXXXX";
    char       *directory_path;
    ft_string   file_path;
    ft_string   long_name;
    int         file_descriptor;
    ssize_t     write_result;
    s_file_sink sink;

    directory_path = mkdtemp(directory_template);
    FT_ASSERT(directory_path != ft_nullptr);
    file_path = directory_path;
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, file_path.get_error());
    file_path += "/";
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, file_path.get_error());
    long_name = ft_string(255, 'a');
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, long_name.get_error());
    file_path += long_name;
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, file_path.get_error());
    file_descriptor = open(file_path.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0644);
    FT_ASSERT(file_descriptor >= 0);
    write_result = write(file_descriptor, "trigger", 7);
    FT_ASSERT_EQ(7, write_result);
    sink.fd = file_descriptor;
    sink.path = ft_string(file_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, sink.path.get_error());
    sink.max_size = 4;
    errno = 0;
    ft_errno = FT_ERR_SUCCESSS;
    ft_log_rotate(&sink);
    FT_ASSERT_EQ(ENAMETOOLONG, errno);
    FT_ASSERT_EQ(file_descriptor, sink.fd);
    write_result = write(sink.fd, "ok", 2);
    FT_ASSERT_EQ(2, write_result);
    close(sink.fd);
    FT_ASSERT_EQ(0, unlink(file_path.c_str()));
    FT_ASSERT_EQ(0, rmdir(directory_path));
    return (1);
}

FT_TEST(test_logger_set_rotation_without_file_sink,
        "ft_log_set_rotation reports missing file sinks")
{
    ft_log_close();
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_log_set_rotation(1024, 2, 60));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, ft_errno);
    return (1);
}

FT_TEST(test_logger_rotation_by_age, "age-based rotation creates an archive")
{
    char            template_path[] = "/tmp/libft_logger_age_XXXXXX";
    int             temp_fd;
    struct utimbuf  timestamps;
    time_t          current_time;
    ft_string       rotated_path;
    int             log_fd;
    ssize_t         read_count;
    char            buffer[512];

    temp_fd = mkstemp(template_path);
    FT_ASSERT(temp_fd >= 0);
    close(temp_fd);
    ft_log_close();
    FT_ASSERT_EQ(0, ft_log_set_file(template_path, 4096));
    FT_ASSERT_EQ(0, ft_log_set_rotation(4096, 1, 1));
    current_time = ::time(ft_nullptr);
    FT_ASSERT(current_time != static_cast<time_t>(-1));
    timestamps.actime = current_time - 5;
    timestamps.modtime = current_time - 5;
    FT_ASSERT_EQ(0, utime(template_path, &timestamps));
    ft_log_info("age-rotation-test");
    ft_log_close();
    rotated_path = ft_string(template_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rotated_path.get_error());
    rotated_path += ".1";
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rotated_path.get_error());
    log_fd = open(rotated_path.c_str(), O_RDONLY);
    FT_ASSERT(log_fd >= 0);
    read_count = read(log_fd, buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(ft_strstr(buffer, "age-rotation-test") != ft_nullptr);
    close(log_fd);
    log_fd = open(template_path, O_RDONLY);
    FT_ASSERT(log_fd >= 0);
    read_count = read(log_fd, buffer, sizeof(buffer));
    FT_ASSERT_EQ(0, read_count);
    close(log_fd);
    unlink(rotated_path.c_str());
    unlink(template_path);
    return (1);
}

FT_TEST(test_logger_rotation_getter_reports_config,
        "ft_log_get_rotation returns the active rotation policy")
{
    char            template_path[] = "/tmp/libft_logger_get_rotation_XXXXXX";
    int             temp_fd;
    size_t          max_size;
    size_t          retention_count;
    unsigned int    max_age_seconds;

    temp_fd = mkstemp(template_path);
    FT_ASSERT(temp_fd >= 0);
    close(temp_fd);
    ft_log_close();
    FT_ASSERT_EQ(0, ft_log_set_file(template_path, 64));
    FT_ASSERT_EQ(0, ft_log_set_rotation(128, 3, 42));
    max_size = 0;
    retention_count = 0;
    max_age_seconds = 0;
    FT_ASSERT_EQ(0, ft_log_get_rotation(&max_size, &retention_count, &max_age_seconds));
    FT_ASSERT_EQ(static_cast<size_t>(128), max_size);
    FT_ASSERT_EQ(static_cast<size_t>(3), retention_count);
    FT_ASSERT_EQ(static_cast<unsigned int>(42), max_age_seconds);
    ft_log_close();
    unlink(template_path);
    return (1);
}

FT_TEST(test_logger_rotation_retention_limit,
        "rotation honors retention count and discards oldest archives")
{
    char        template_path[] = "/tmp/libft_logger_retention_XXXXXX";
    int         temp_fd;
    ft_string   rotated_one_path;
    ft_string   rotated_two_path;
    ft_string   rotated_three_path;
    int         fd;
    ssize_t     read_count;
    char        buffer[512];

    temp_fd = mkstemp(template_path);
    FT_ASSERT(temp_fd >= 0);
    close(temp_fd);
    ft_log_close();
    FT_ASSERT_EQ(0, ft_log_set_file(template_path, 8));
    FT_ASSERT_EQ(0, ft_log_set_rotation(8, 2, 0));
    ft_log_info("retention-first");
    ft_log_info("retention-second");
    ft_log_info("retention-third");
    ft_log_close();
    rotated_one_path = ft_string(template_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rotated_one_path.get_error());
    rotated_one_path += ".1";
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rotated_one_path.get_error());
    rotated_two_path = ft_string(template_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rotated_two_path.get_error());
    rotated_two_path += ".2";
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rotated_two_path.get_error());
    rotated_three_path = ft_string(template_path);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rotated_three_path.get_error());
    rotated_three_path += ".3";
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, rotated_three_path.get_error());
    fd = open(rotated_one_path.c_str(), O_RDONLY);
    FT_ASSERT(fd >= 0);
    read_count = read(fd, buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(ft_strstr(buffer, "retention-third") != ft_nullptr);
    close(fd);
    fd = open(rotated_two_path.c_str(), O_RDONLY);
    FT_ASSERT(fd >= 0);
    read_count = read(fd, buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(ft_strstr(buffer, "retention-second") != ft_nullptr);
    close(fd);
    errno = 0;
    FT_ASSERT_EQ(-1, access(rotated_three_path.c_str(), F_OK));
    FT_ASSERT_EQ(FT_ERR_IO, cmp_map_system_error_to_ft(errno));
    unlink(rotated_one_path.c_str());
    unlink(rotated_two_path.c_str());
    unlink(template_path);
    return (1);
}

