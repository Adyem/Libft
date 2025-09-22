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

        void set_error(int error_code) const noexcept;
        int configure_listener(uint16_t port);
        int connect_client(uint16_t port);
        int accept_client();
        int set_blocking_flag(int file_descriptor, bool should_block);
        void cleanup_reader();
        void reader_loop();
        static void reader_entry(network_io_harness *harness);

    public:
        network_io_harness();
        ~network_io_harness();

        int initialize(uint16_t port);
        void shutdown();

        int get_client_fd() const;
        int get_server_fd() const;
        ft_socket &get_client_socket();

        int set_blocking(bool should_block);
        int enable_non_blocking();
        int enable_blocking();

        int start_throttled_reads(size_t throttle_bytes, size_t delay_microseconds);
        void stop_throttled_reads();
        void close_client();

        int get_error() const;
        const char *get_error_str() const;
};

#endif
