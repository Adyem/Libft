#ifndef NETWORK_IO_HARNESS_HPP
#define NETWORK_IO_HARNESS_HPP

#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../PThread/thread.hpp"
#include "../../Errno/errno.hpp"
#include <cstddef>
#include <cstdint>

class network_io_harness
{
    private:
        mutable int _error_code;
        ft_socket _listener_socket;
        ft_socket _client_socket;
        int _accepted_fd;
        struct sockaddr_storage _accepted_address;
        ft_thread *_reader_thread;
        int _stop_reader;
        size_t _throttle_bytes;
        size_t _throttle_delay_us;
        int _reader_fd;
        int _mode;

        enum harness_mode
        {
            HARNESS_MODE_IDLE = 0,
            HARNESS_MODE_LOOPBACK = 1,
            HARNESS_MODE_REMOTE = 2
        };

        void set_error(int error_code) const noexcept;
        int configure_listener(uint16_t port);
        int connect_client(uint16_t port);
        int accept_client();
        int connect_remote_client(const char *ip_address, uint16_t port, int address_family);
        int set_blocking_flag(int file_descriptor, bool should_block);
        int set_socket_buffer(int file_descriptor, int option_name, size_t size);
        void cleanup_reader();
        void reader_loop();
        int start_reader_on_descriptor(int descriptor, size_t throttle_bytes, size_t delay_microseconds);
        int shutdown_descriptor(int descriptor, int how);
        static void reader_entry(network_io_harness *harness);

    public:
        network_io_harness();
        ~network_io_harness();

        int initialize(uint16_t port);
        int connect_remote(const char *ip_address, uint16_t port);
        int connect_remote(const char *ip_address, uint16_t port, int address_family);
        void shutdown();

        int get_client_fd() const;
        int get_server_fd() const;
        int get_listener_fd() const;
        const struct sockaddr_storage &get_server_address() const;
        uint16_t get_listener_port() const;
        ft_socket &get_client_socket();

        int set_blocking(bool should_block);
        int set_client_blocking(bool should_block);
        int set_server_blocking(bool should_block);
        int enable_non_blocking();
        int enable_blocking();

        int set_client_send_buffer(size_t buffer_size);
        int set_server_send_buffer(size_t buffer_size);
        int set_client_receive_buffer(size_t buffer_size);
        int set_server_receive_buffer(size_t buffer_size);

        int start_throttled_reads(size_t throttle_bytes, size_t delay_microseconds);
        int start_throttled_reads_on_server(size_t throttle_bytes, size_t delay_microseconds);
        void stop_throttled_reads();
        void close_client();
        void close_server();
        int shutdown_client_read();
        int shutdown_client_write();
        int shutdown_client_both();
        int shutdown_server_read();
        int shutdown_server_write();
        int shutdown_server_both();

        int get_error() const;
        const char *get_error_str() const;
};

#endif
