#include "../test_internal.hpp"
#include "../../Modules/Networking/networking.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_networking_socket_config_prepare_rejects_null)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, socket_config_prepare_thread_safety(ft_nullptr));
    return (1);
}

FT_TEST(test_networking_socket_config_prepare_accepts_valid_config)
{
    SocketConfig config;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, config.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, socket_config_prepare_thread_safety(&config));
    return (1);
}

FT_TEST(test_networking_socket_config_lock_rejects_null)
{
    ft_bool lock_acquired;

    lock_acquired = FT_TRUE;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, socket_config_lock(ft_nullptr, &lock_acquired));
    FT_ASSERT_EQ(FT_FALSE, lock_acquired);
    return (1);
}

FT_TEST(test_networking_socket_config_lock_sets_lock_acquired)
{
    SocketConfig config;
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, config.initialize());
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, socket_config_lock(&config, &lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    socket_config_unlock(&config, lock_acquired);
    return (1);
}

FT_TEST(test_networking_socket_config_teardown_accepts_valid_config)
{
    SocketConfig config;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, config.initialize());
    socket_config_teardown_thread_safety(&config);
    return (1);
}
