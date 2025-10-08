#include "../../Networking/networking.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include <utility>

FT_TEST(test_socket_config_copy_detects_string_error,
    "SocketConfig copy constructor captures ft_string error state")
{
    SocketConfig original;
    const char *error_string;

    original._ip = ft_string(FT_ERR_INVALID_ARGUMENT);
    original._multicast_group = "239.1.1.1";
    ft_errno = ER_SUCCESS;
    SocketConfig copy(original);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, copy.get_error());
    error_string = copy.get_error_str();
    FT_ASSERT(error_string != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(error_string, ft_strerror(FT_ERR_INVALID_ARGUMENT)));
    return (1);
}

FT_TEST(test_socket_config_move_resets_source_fields,
    "SocketConfig move constructor transfers values and clears source")
{
    SocketConfig original;

    original._type = SocketType::CLIENT;
    original._ip = "10.1.2.3";
    original._port = 4242;
    original._backlog = 4;
    original._protocol = IPPROTO_UDP;
    original._address_family = AF_INET6;
    original._reuse_address = false;
    original._non_blocking = true;
    original._recv_timeout = 1000;
    original._send_timeout = 2000;
    original._multicast_group = "239.0.0.1";
    original._multicast_interface = "eth0";
    ft_errno = ER_SUCCESS;
    SocketConfig moved(std::move(original));
    FT_ASSERT_EQ(SocketType::CLIENT, moved._type);
    FT_ASSERT_EQ(0, ft_strcmp(moved._ip.c_str(), "10.1.2.3"));
    FT_ASSERT_EQ(4242, moved._port);
    FT_ASSERT_EQ(4, moved._backlog);
    FT_ASSERT_EQ(IPPROTO_UDP, moved._protocol);
    FT_ASSERT_EQ(AF_INET6, moved._address_family);
    FT_ASSERT_EQ(false, moved._reuse_address);
    FT_ASSERT_EQ(true, moved._non_blocking);
    FT_ASSERT_EQ(1000, moved._recv_timeout);
    FT_ASSERT_EQ(2000, moved._send_timeout);
    FT_ASSERT_EQ(0, ft_strcmp(moved._multicast_group.c_str(), "239.0.0.1"));
    FT_ASSERT_EQ(0, ft_strcmp(moved._multicast_interface.c_str(), "eth0"));
    FT_ASSERT_EQ(SocketType::CLIENT, original._type);
    FT_ASSERT_EQ(0, original._port);
    FT_ASSERT_EQ(0, original._backlog);
    FT_ASSERT_EQ(0, original._protocol);
    FT_ASSERT_EQ(0, original._address_family);
    FT_ASSERT_EQ(false, original._reuse_address);
    FT_ASSERT_EQ(false, original._non_blocking);
    FT_ASSERT_EQ(0, original._recv_timeout);
    FT_ASSERT_EQ(0, original._send_timeout);
    FT_ASSERT_EQ(0, ft_strcmp(original._multicast_group.c_str(), ""));
    FT_ASSERT_EQ(0, ft_strcmp(original._multicast_interface.c_str(), ""));
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
