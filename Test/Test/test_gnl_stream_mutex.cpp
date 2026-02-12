#include "../test_internal.hpp"
#include "../../GetNextLine/gnl_stream.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <cstdio>

#ifndef LIBFT_TEST_BUILD
#endif

static ssize_t gnl_stream_mutex_callback_success(void *, char *buffer, size_t max_size) noexcept
{
    if (buffer == ft_nullptr || max_size == 0)
        return (-1);
    buffer[0] = 'x';
    return (1);
}

static ssize_t gnl_stream_mutex_callback_failure(void *, char *, size_t) noexcept
{
    return (-1);
}

static int gnl_stream_expect_mutex_unlocked(gnl_stream &stream_instance)
{
    pt_recursive_mutex *mutex_pointer;
    int lock_error;
    int unlock_error;

    mutex_pointer = stream_instance.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    lock_error = mutex_pointer->lock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error);
    lock_error = mutex_pointer->lock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error);
    FT_ASSERT_EQ(true, mutex_pointer->lockState());
    unlock_error = mutex_pointer->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error);
    unlock_error = mutex_pointer->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (1);
}

FT_TEST(test_gnl_stream_initialize_unlocks_mutex,
    "gnl_stream initialize prepares unlocked recursive mutex")
{
    gnl_stream stream_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(1, gnl_stream_expect_mutex_unlocked(stream_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_init_from_fd_unlocks_mutex_success,
    "gnl_stream init_from_fd unlocks mutex on success")
{
    gnl_stream stream_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.init_from_fd(0));
    FT_ASSERT_EQ(1, gnl_stream_expect_mutex_unlocked(stream_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_init_from_fd_unlocks_mutex_failure,
    "gnl_stream init_from_fd unlocks mutex on invalid descriptor")
{
    gnl_stream stream_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, stream_instance.init_from_fd(-1));
    FT_ASSERT_EQ(1, gnl_stream_expect_mutex_unlocked(stream_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_init_from_file_unlocks_mutex_failure,
    "gnl_stream init_from_file unlocks mutex on null file")
{
    gnl_stream stream_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, stream_instance.init_from_file(ft_nullptr, false));
    FT_ASSERT_EQ(1, gnl_stream_expect_mutex_unlocked(stream_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_init_from_callback_unlocks_mutex_failure,
    "gnl_stream init_from_callback unlocks mutex on null callback")
{
    gnl_stream stream_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
        stream_instance.init_from_callback(ft_nullptr, ft_nullptr));
    FT_ASSERT_EQ(1, gnl_stream_expect_mutex_unlocked(stream_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_read_unlocks_mutex_invalid_arguments,
    "gnl_stream read unlocks mutex on invalid arguments")
{
    gnl_stream stream_instance;
    char buffer[2];

    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(-1, stream_instance.read(buffer, 0));
    FT_ASSERT_EQ(1, gnl_stream_expect_mutex_unlocked(stream_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_read_unlocks_mutex_invalid_state,
    "gnl_stream read unlocks mutex when no source is configured")
{
    gnl_stream stream_instance;
    char buffer[2];

    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(-1, stream_instance.read(buffer, 1));
    FT_ASSERT_EQ(1, gnl_stream_expect_mutex_unlocked(stream_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_read_unlocks_mutex_callback_failure,
    "gnl_stream read unlocks mutex when callback fails")
{
    gnl_stream stream_instance;
    char buffer[2];

    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        stream_instance.init_from_callback(gnl_stream_mutex_callback_failure, ft_nullptr));
    FT_ASSERT_EQ(-1, stream_instance.read(buffer, 1));
    FT_ASSERT_EQ(1, gnl_stream_expect_mutex_unlocked(stream_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_read_unlocks_mutex_callback_success,
    "gnl_stream read unlocks mutex when callback succeeds")
{
    gnl_stream stream_instance;
    char buffer[2];

    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        stream_instance.init_from_callback(gnl_stream_mutex_callback_success, ft_nullptr));
    FT_ASSERT_EQ(1, stream_instance.read(buffer, 1));
    FT_ASSERT_EQ(1, gnl_stream_expect_mutex_unlocked(stream_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_reset_unlocks_mutex,
    "gnl_stream reset leaves recursive mutex unlocked")
{
    gnl_stream stream_instance;
    FILE *file_pointer;

    file_pointer = tmpfile();
    if (file_pointer == ft_nullptr)
        return (1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.init_from_file(file_pointer, true));
    stream_instance.reset();
    FT_ASSERT_EQ(1, gnl_stream_expect_mutex_unlocked(stream_instance));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}

FT_TEST(test_gnl_stream_failure_paths_leave_mutex_unlocked,
    "gnl_stream failure paths leave recursive mutex unlocked")
{
    gnl_stream stream_instance;
    pt_recursive_mutex *mutex_pointer;
    char buffer[2];

    buffer[0] = '\0';
    buffer[1] = '\0';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, stream_instance.init_from_fd(-1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
        stream_instance.init_from_callback(ft_nullptr, ft_nullptr));
    FT_ASSERT_EQ(-1, stream_instance.read(buffer, 0));
    mutex_pointer = stream_instance.get_mutex_for_validation();
    FT_ASSERT_EQ(1, gnl_stream_expect_mutex_unlocked(stream_instance));
    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stream_instance.destroy());
    return (1);
}
