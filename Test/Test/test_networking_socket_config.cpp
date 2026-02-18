#include "../test_internal.hpp"
#include "../../Networking/networking.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_socket_config_initialize_sets_defaults,
    "SocketConfig initialize populates default values")
{
    SocketConfig configuration;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, configuration.initialize());
    FT_ASSERT_EQ(SocketType::SERVER, configuration._type);
    FT_ASSERT_EQ(0, ft_strcmp(configuration._ip, "127.0.0.1"));
    FT_ASSERT_EQ(8080, configuration._port);
    FT_ASSERT_EQ(10, configuration._backlog);
    FT_ASSERT_EQ(IPPROTO_TCP, configuration._protocol);
    FT_ASSERT_EQ(AF_INET, configuration._address_family);
    FT_ASSERT_EQ(true, configuration._reuse_address);
    FT_ASSERT_EQ(false, configuration._non_blocking);
    FT_ASSERT_EQ(5000, configuration._recv_timeout);
    FT_ASSERT_EQ(5000, configuration._send_timeout);
    FT_ASSERT_EQ(0, ft_strcmp(configuration._multicast_group, ""));
    FT_ASSERT_EQ(0, ft_strcmp(configuration._multicast_interface, ""));
    return (1);
}

FT_TEST(test_socket_config_destroy_resets_fields,
    "SocketConfig destroy resets mutable fields")
{
    SocketConfig configuration;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, configuration.initialize());
    std::strncpy(configuration._ip, "10.1.2.3", sizeof(configuration._ip) - 1);
    configuration._ip[sizeof(configuration._ip) - 1] = '\0';
    configuration._port = 4242;
    configuration._backlog = 4;
    configuration._protocol = IPPROTO_UDP;
    configuration._address_family = AF_INET6;
    configuration._reuse_address = false;
    configuration._non_blocking = true;
    configuration._recv_timeout = 1000;
    configuration._send_timeout = 2000;
    std::strncpy(configuration._multicast_group, "239.0.0.1",
        sizeof(configuration._multicast_group) - 1);
    configuration._multicast_group[sizeof(configuration._multicast_group) - 1] = '\0';
    std::strncpy(configuration._multicast_interface, "eth0",
        sizeof(configuration._multicast_interface) - 1);
    configuration._multicast_interface[sizeof(configuration._multicast_interface) - 1] = '\0';
    FT_ASSERT_EQ(FT_ERR_SUCCESS, configuration.destroy());
    FT_ASSERT_EQ(0, ft_strcmp(configuration._ip, ""));
    FT_ASSERT_EQ(0, configuration._port);
    FT_ASSERT_EQ(0, configuration._backlog);
    FT_ASSERT_EQ(0, configuration._protocol);
    FT_ASSERT_EQ(0, configuration._address_family);
    FT_ASSERT_EQ(false, configuration._reuse_address);
    FT_ASSERT_EQ(false, configuration._non_blocking);
    FT_ASSERT_EQ(0, configuration._recv_timeout);
    FT_ASSERT_EQ(0, configuration._send_timeout);
    FT_ASSERT_EQ(0, ft_strcmp(configuration._multicast_group, ""));
    FT_ASSERT_EQ(0, ft_strcmp(configuration._multicast_interface, ""));
    return (1);
}
