#include "../../Networking/networking.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <utility>

FT_TEST(test_socket_config_prepare_initializes_mutex,
    "socket_config_prepare_thread_safety initializes mutex and allows locking")
{
    SocketConfig config;
    bool lock_acquired;

    socket_config_teardown_thread_safety(&config);
    ft_errno = FT_ERR_INTERNAL;
    FT_ASSERT_EQ(0, socket_config_prepare_thread_safety(&config));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    lock_acquired = false;
    FT_ASSERT_EQ(0, socket_config_lock(&config, &lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    ft_errno = FT_ERR_SOCKET_BIND_FAILED;
    socket_config_unlock(&config, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SOCKET_BIND_FAILED, ft_errno);
    socket_config_teardown_thread_safety(&config);
    return (1);
}

FT_TEST(test_socket_config_prepare_rejects_null,
    "socket_config_prepare_thread_safety validates null arguments")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, socket_config_prepare_thread_safety(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_socket_config_prepare_is_idempotent,
    "socket_config_prepare_thread_safety is idempotent when already prepared")
{
    SocketConfig config;
    bool lock_acquired;

    FT_ASSERT_EQ(0, socket_config_prepare_thread_safety(&config));
    lock_acquired = false;
    FT_ASSERT_EQ(0, socket_config_lock(&config, &lock_acquired));
    FT_ASSERT(lock_acquired);
    socket_config_unlock(&config, lock_acquired);
    ft_errno = FT_ERR_INTERNAL;
    lock_acquired = false;
    FT_ASSERT_EQ(0, socket_config_prepare_thread_safety(&config));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, socket_config_lock(&config, &lock_acquired));
    FT_ASSERT(lock_acquired);
    socket_config_unlock(&config, lock_acquired);
    socket_config_teardown_thread_safety(&config);
    return (1);
}

FT_TEST(test_socket_config_lock_rejects_null,
    "socket_config_lock validates null arguments")
{
    bool lock_acquired;

    lock_acquired = true;
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, socket_config_lock(ft_nullptr, &lock_acquired));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_socket_config_lock_handles_disabled_safety,
    "socket_config_lock returns without locking when thread safety disabled")
{
    SocketConfig config;
    bool lock_acquired;

    socket_config_teardown_thread_safety(&config);
    lock_acquired = true;
    FT_ASSERT_EQ(0, socket_config_lock(&config, &lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    socket_config_unlock(&config, lock_acquired);
    return (1);
}

FT_TEST(test_socket_config_unlock_resets_errno_without_lock,
    "socket_config_unlock sets errno to success when lock not held")
{
    SocketConfig config;

    socket_config_teardown_thread_safety(&config);
    ft_errno = FT_ERR_SOCKET_ACCEPT_FAILED;
    socket_config_unlock(&config, false);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_socket_config_copy_has_independent_mutex,
    "SocketConfig copy constructor allocates independent mutex")
{
    SocketConfig original;
    SocketConfig copy(original);
    bool original_locked;
    bool copy_locked;

    original_locked = false;
    FT_ASSERT_EQ(0, socket_config_lock(&original, &original_locked));
    FT_ASSERT_EQ(true, original_locked);
    copy_locked = false;
    FT_ASSERT_EQ(0, socket_config_lock(&copy, &copy_locked));
    FT_ASSERT_EQ(true, copy_locked);
    socket_config_unlock(&copy, copy_locked);
    socket_config_unlock(&original, original_locked);
    return (1);
}

FT_TEST(test_socket_config_teardown_resets_state,
    "socket_config_teardown_thread_safety frees mutex and disables locking")
{
    SocketConfig config;
    bool lock_acquired;

    FT_ASSERT_EQ(0, socket_config_prepare_thread_safety(&config));
    lock_acquired = false;
    FT_ASSERT_EQ(0, socket_config_lock(&config, &lock_acquired));
    FT_ASSERT(lock_acquired);
    socket_config_unlock(&config, lock_acquired);
    socket_config_teardown_thread_safety(&config);
    lock_acquired = true;
    FT_ASSERT_EQ(0, socket_config_lock(&config, &lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    ft_errno = FT_ERR_SOCKET_BIND_FAILED;
    socket_config_teardown_thread_safety(&config);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_socket_config_move_preserves_mutex_validity,
    "SocketConfig move constructor leaves both instances lockable")
{
    SocketConfig source;
    SocketConfig moved(std::move(source));
    bool moved_locked;
    bool source_locked;

    moved_locked = false;
    FT_ASSERT_EQ(0, socket_config_lock(&moved, &moved_locked));
    FT_ASSERT_EQ(true, moved_locked);
    source_locked = false;
    FT_ASSERT_EQ(0, socket_config_lock(&source, &source_locked));
    FT_ASSERT_EQ(true, source_locked);
    socket_config_unlock(&source, source_locked);
    socket_config_unlock(&moved, moved_locked);
    return (1);
}
