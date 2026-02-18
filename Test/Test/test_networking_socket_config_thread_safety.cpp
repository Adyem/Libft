#include "../test_internal.hpp"
#include "../../Networking/networking.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_socket_config_prepare_rejects_null,
    "socket_config_prepare_thread_safety validates null arguments")
{
    FT_ASSERT_EQ(-1, socket_config_prepare_thread_safety(ft_nullptr));
    return (1);
}

FT_TEST(test_socket_config_prepare_accepts_valid_config,
    "socket_config_prepare_thread_safety succeeds for initialized config")
{
    SocketConfig config;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, config.initialize());
    FT_ASSERT_EQ(0, socket_config_prepare_thread_safety(&config));
    return (1);
}

FT_TEST(test_socket_config_lock_rejects_null,
    "socket_config_lock validates null arguments")
{
    bool lock_acquired;

    lock_acquired = true;
    FT_ASSERT_EQ(-1, socket_config_lock(ft_nullptr, &lock_acquired));
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_socket_config_lock_sets_lock_acquired,
    "socket_config_lock reports acquired state for valid config")
{
    SocketConfig config;
    bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, config.initialize());
    lock_acquired = false;
    FT_ASSERT_EQ(0, socket_config_lock(&config, &lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    socket_config_unlock(&config, lock_acquired);
    return (1);
}

FT_TEST(test_socket_config_teardown_accepts_valid_config,
    "socket_config_teardown_thread_safety is safe for initialized config")
{
    SocketConfig config;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, config.initialize());
    socket_config_teardown_thread_safety(&config);
    return (1);
}
