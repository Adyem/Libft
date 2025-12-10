#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../PThread/pthread.hpp"
#include <thread>
#include <atomic>
#include <cstring>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# define CLOSE_SOCKET closesocket
#else
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
# define CLOSE_SOCKET close
#endif

static int networking_socket_create_server(uint16_t &port)
{
    int server_fd;
    struct sockaddr_in address;
    socklen_t length;

    server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        return (-1);
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;
    if (::bind(server_fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) != 0)
    {
        CLOSE_SOCKET(server_fd);
        return (-1);
    }
    if (::listen(server_fd, 4) != 0)
    {
        CLOSE_SOCKET(server_fd);
        return (-1);
    }
    length = sizeof(address);
    if (::getsockname(server_fd, reinterpret_cast<struct sockaddr*>(&address), &length) != 0)
    {
        CLOSE_SOCKET(server_fd);
        return (-1);
    }
    port = ntohs(address.sin_port);
    return (server_fd);
}

static void networking_socket_configure_client(SocketConfig &config, uint16_t port)
{
    config._type = SocketType::CLIENT;
    config._ip = "127.0.0.1";
    config._port = port;
    config._address_family = AF_INET;
    config._non_blocking = false;
    config._recv_timeout = 0;
    config._send_timeout = 0;
    return ;
}

FT_TEST(test_ft_socket_send_all_thread_safety,
    "ft_socket send_all remains consistent under concurrent inspection")
{
    uint16_t server_port;
    int server_fd;
    SocketConfig client_config;
    ft_socket client_socket;
    std::thread accept_thread;
    int accepted_fd;
    std::atomic<bool> inspector_running;
    std::thread inspector_thread;
    std::thread send_thread;
    std::thread reader_thread;
    const char message[] = "pingdata";
    int message_length;
    int send_iterations;
    std::atomic<int> received_total;
    std::atomic<bool> thread_failed;

    server_fd = networking_socket_create_server(server_port);
    FT_ASSERT(server_fd >= 0);
    networking_socket_configure_client(client_config, server_port);
    accepted_fd = -1;
    accept_thread = std::thread([server_fd, &accepted_fd]() {
        struct sockaddr_in client_addr;
        socklen_t length;

        std::memset(&client_addr, 0, sizeof(client_addr));
        length = sizeof(client_addr);
        accepted_fd = ::accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &length);
        return ;
    });
    client_socket = ft_socket(client_config);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, client_socket.get_error());
    accept_thread.join();
    FT_ASSERT(accepted_fd >= 0);
    message_length = static_cast<int>(sizeof(message) - 1);
    send_iterations = 60;
    received_total.store(0);
    thread_failed.store(false);
    reader_thread = std::thread([accepted_fd, message_length, send_iterations, &received_total]() {
        char buffer[128];
        int expected_total;
        int local_total;

        expected_total = message_length * send_iterations;
        local_total = 0;
        while (local_total < expected_total)
        {
            ssize_t bytes_read;

            bytes_read = ::recv(accepted_fd, buffer, sizeof(buffer), 0);
            if (bytes_read > 0)
                local_total += static_cast<int>(bytes_read);
        }
        received_total.store(local_total);
        return ;
    });
    inspector_running.store(true);
    inspector_thread = std::thread([&client_socket, &inspector_running, &thread_failed]() {
        while (inspector_running.load())
        {
            int descriptor;
            const char *error_string;

            descriptor = client_socket.get_fd();
            if (descriptor < 0)
            {
                thread_failed.store(true);
                return ;
            }
            error_string = client_socket.get_error_str();
            if (error_string == ft_nullptr)
            {
                thread_failed.store(true);
                return ;
            }
        }
        return ;
    });
    send_thread = std::thread([&client_socket, message, message_length, send_iterations, &thread_failed]() {
        int iteration;

        iteration = 0;
        while (iteration < send_iterations)
        {
            ssize_t bytes_sent;

            bytes_sent = client_socket.send_all(message, message_length, 0);
            if (static_cast<int>(bytes_sent) != message_length)
            {
                thread_failed.store(true);
                return ;
            }
            if (client_socket.get_error() != FT_ER_SUCCESSS)
            {
                thread_failed.store(true);
                return ;
            }
            iteration++;
        }
        return ;
    });
    send_thread.join();
    while (received_total.load() < message_length * send_iterations)
        pt_thread_sleep(1);
    inspector_running.store(false);
    inspector_thread.join();
    reader_thread.join();
    FT_ASSERT(thread_failed.load() == false);
    FT_ASSERT_EQ(message_length * send_iterations, received_total.load());
    CLOSE_SOCKET(accepted_fd);
    CLOSE_SOCKET(server_fd);
    client_socket.close_socket();
    return (1);
}

FT_TEST(test_ft_socket_receive_close_thread_safety,
    "ft_socket receive_data handles concurrent closure")
{
    uint16_t server_port;
    int server_fd;
    SocketConfig client_config;
    ft_socket client_socket;
    std::thread accept_thread;
    int accepted_fd;
    std::thread sender_thread;
    std::thread receiver_thread;
    std::atomic<bool> received_once;
    std::atomic<bool> close_requested;
    std::thread closer_thread;

    server_fd = networking_socket_create_server(server_port);
    FT_ASSERT(server_fd >= 0);
    networking_socket_configure_client(client_config, server_port);
    accepted_fd = -1;
    accept_thread = std::thread([server_fd, &accepted_fd]() {
        struct sockaddr_in client_addr;
        socklen_t length;

        std::memset(&client_addr, 0, sizeof(client_addr));
        length = sizeof(client_addr);
        accepted_fd = ::accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &length);
        return ;
    });
    client_socket = ft_socket(client_config);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, client_socket.get_error());
    accept_thread.join();
    FT_ASSERT(accepted_fd >= 0);
    received_once.store(false);
    close_requested.store(false);
    sender_thread = std::thread([accepted_fd, &close_requested]() {
        const char payload[] = "closure-test-payload";
        int iteration;

        iteration = 0;
        while (iteration < 40 && !close_requested.load())
        {
            ::send(accepted_fd, payload, sizeof(payload) - 1, 0);
            pt_thread_sleep(1);
            iteration++;
        }
        return ;
    });
    receiver_thread = std::thread([&client_socket, &received_once]() {
        char buffer[64];
        int loop_index;

        loop_index = 0;
        while (loop_index < 80)
        {
            ssize_t result;

            result = client_socket.receive_data(buffer, sizeof(buffer), 0);
            if (result > 0)
                received_once.store(true);
            if (result < 0)
                break;
            loop_index++;
        }
        return ;
    });
    closer_thread = std::thread([&client_socket, &received_once, &close_requested]() {
        int wait_loops;

        wait_loops = 0;
        while (!received_once.load() && wait_loops < 200)
        {
            pt_thread_sleep(1);
            wait_loops++;
        }
        close_requested.store(true);
        client_socket.close_socket();
        return ;
    });
    receiver_thread.join();
    closer_thread.join();
    sender_thread.join();
    CLOSE_SOCKET(accepted_fd);
    CLOSE_SOCKET(server_fd);
    client_socket.close_socket();
    return (1);
}
