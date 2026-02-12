#include "../test_internal.hpp"
#include "../../CrossProcess/cross_process.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>

#ifndef _WIN32
#include "../../CPP_class/class_nullptr.hpp"
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace
{
    static int create_shared_memory(const char *name_prefix, const char *payload, size_t payload_length, cross_process_message &message,
        void *&mapping_ptr, unsigned char *&mapping, size_t &data_offset, size_t &error_offset)
    {
        char shared_memory_name[64];
        int shm_fd;
        size_t total_size;
        pthread_mutex_t *shared_mutex;
        pthread_mutexattr_t mutex_attributes;

        data_offset = sizeof(pthread_mutex_t);
        error_offset = data_offset + payload_length;
        total_size = error_offset + sizeof(int);
        std::snprintf(shared_memory_name, sizeof(shared_memory_name), "%s_%ld", name_prefix, static_cast<long>(getpid()));
        shm_fd = shm_open(shared_memory_name, O_CREAT | O_RDWR, 0600);
        if (shm_fd < 0)
            return (-1);
        if (ftruncate(shm_fd, static_cast<off_t>(total_size)) != 0)
        {
            ::close(shm_fd);
            return (-1);
        }
        mapping_ptr = mmap(ft_nullptr, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (mapping_ptr == MAP_FAILED)
        {
            ::close(shm_fd);
            return (-1);
        }
        mapping = reinterpret_cast<unsigned char *>(mapping_ptr);
        if (::close(shm_fd) != 0)
            return (-1);
        std::memset(mapping, 0, total_size);
        shared_mutex = reinterpret_cast<pthread_mutex_t *>(mapping);
        if (pthread_mutexattr_init(&mutex_attributes) != 0)
            return (-1);
        if (pthread_mutexattr_setpshared(&mutex_attributes, PTHREAD_PROCESS_SHARED) != 0)
        {
            pthread_mutexattr_destroy(&mutex_attributes);
            return (-1);
        }
        if (pthread_mutex_init(shared_mutex, &mutex_attributes) != 0)
        {
            pthread_mutexattr_destroy(&mutex_attributes);
            return (-1);
        }
        if (pthread_mutexattr_destroy(&mutex_attributes) != 0)
            return (-1);
        std::memcpy(mapping + data_offset, payload, payload_length);
        *reinterpret_cast<int *>(mapping + error_offset) = 123;
        std::memset(&message, 0, sizeof(message));
        message.stack_base_address = reinterpret_cast<uint64_t>(mapping);
        message.remote_memory_address = message.stack_base_address + static_cast<uint64_t>(data_offset);
        message.remote_memory_size = static_cast<uint64_t>(total_size);
        message.shared_mutex_address = message.stack_base_address;
        message.error_memory_address = message.stack_base_address + static_cast<uint64_t>(error_offset);
        std::snprintf(message.shared_memory_name, sizeof(message.shared_memory_name), "%s", shared_memory_name);
        return (0);
    }
}

FT_TEST(test_cross_process_receive_memory_basic, "cross process receive memory basic")
{
    cross_process_message message;
    cross_process_read_result result;
    void *mapping_ptr;
    unsigned char *mapping;
    size_t data_offset;
    size_t error_offset;
    int sockets[2];
    int send_result;
    int receive_result;
    int close_result;
    size_t index;
    const char *payload;
    size_t payload_length;

    payload = "cross process payload";
    payload_length = std::strlen(payload);
    mapping_ptr = ft_nullptr;
    mapping = ft_nullptr;
    data_offset = 0;
    error_offset = 0;
    FT_ASSERT_EQ(0, create_shared_memory("/cross_process_test", payload, payload_length, message, mapping_ptr, mapping, data_offset, error_offset));
    FT_ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
    send_result = cp_send_descriptor(sockets[0], message);
    FT_ASSERT_EQ(0, send_result);
    receive_result = cp_receive_memory(sockets[1], result);
    FT_ASSERT_EQ(0, receive_result);
    close_result = ::close(sockets[0]);
    FT_ASSERT_EQ(0, close_result);
    close_result = ::close(sockets[1]);
    FT_ASSERT_EQ(0, close_result);
    FT_ASSERT(result.shared_memory_name == message.shared_memory_name);
    FT_ASSERT_EQ(message.remote_memory_size - data_offset, result.payload.size());
    FT_ASSERT(std::memcmp(result.payload.data(), payload, payload_length) == 0);
    int captured_error_value;

    std::memcpy(&captured_error_value, result.payload.data() + payload_length, sizeof(int));
    FT_ASSERT_EQ(123, captured_error_value);
    index = 0;
    while (index < payload_length)
    {
        FT_ASSERT_EQ(0, mapping[data_offset + index]);
        index++;
    }
    int post_error_value;

    std::memcpy(&post_error_value, mapping + error_offset, sizeof(int));
    FT_ASSERT_EQ(0, post_error_value);
    FT_ASSERT_EQ(0, pthread_mutex_destroy(reinterpret_cast<pthread_mutex_t *>(mapping)));
    FT_ASSERT_EQ(0, munmap(mapping_ptr, static_cast<size_t>(message.remote_memory_size)));
    FT_ASSERT_EQ(0, shm_unlink(message.shared_memory_name));
    return (1);
}

FT_TEST(test_cross_process_receive_memory_mutex_timeout, "cross process receive memory mutex timeout")
{
    cross_process_message message;
    void *mapping_ptr;
    unsigned char *mapping;
    size_t data_offset;
    size_t error_offset;
    int sockets[2];
    int send_result;
    int receive_result;
    int close_result;
    pthread_mutex_t *shared_mutex;

    mapping_ptr = ft_nullptr;
    mapping = ft_nullptr;
    data_offset = 0;
    error_offset = 0;
    FT_ASSERT_EQ(0, create_shared_memory("/cross_process_mutex", "payload", std::strlen("payload"), message, mapping_ptr, mapping, data_offset, error_offset));
    shared_mutex = reinterpret_cast<pthread_mutex_t *>(mapping);
    FT_ASSERT_EQ(0, pthread_mutex_lock(shared_mutex));
    FT_ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
    send_result = cp_send_descriptor(sockets[0], message);
    FT_ASSERT_EQ(0, send_result);
    cross_process_read_result timeout_result;

    errno = 0;
    receive_result = cp_receive_memory(sockets[1], timeout_result);
    FT_ASSERT_EQ(-1, receive_result);
    FT_ASSERT_EQ(ETIMEDOUT, errno);
    close_result = ::close(sockets[0]);
    FT_ASSERT_EQ(0, close_result);
    close_result = ::close(sockets[1]);
    FT_ASSERT_EQ(0, close_result);
    FT_ASSERT_EQ(0, pthread_mutex_unlock(shared_mutex));
    FT_ASSERT_EQ(0, pthread_mutex_destroy(shared_mutex));
    FT_ASSERT_EQ(0, munmap(mapping_ptr, static_cast<size_t>(message.remote_memory_size)));
    FT_ASSERT_EQ(0, shm_unlink(message.shared_memory_name));
    return (1);
}

FT_TEST(test_cross_process_receive_memory_invalid_payload_offset, "cross process receive memory invalid payload offset")
{
    cross_process_message message;
    cross_process_read_result result;
    void *mapping_ptr;
    unsigned char *mapping;
    size_t data_offset;
    size_t error_offset;
    int sockets[2];
    int send_result;
    int receive_result;
    int close_result;
    const char *payload;
    size_t payload_length;
    size_t index;

    payload = "payload";
    payload_length = std::strlen(payload);
    mapping_ptr = ft_nullptr;
    mapping = ft_nullptr;
    data_offset = 0;
    error_offset = 0;
    FT_ASSERT_EQ(0, create_shared_memory("/cross_process_invalid_payload", payload, payload_length, message, mapping_ptr, mapping,
        data_offset, error_offset));
    message.remote_memory_address = message.stack_base_address + message.remote_memory_size;
    FT_ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
    send_result = cp_send_descriptor(sockets[0], message);
    FT_ASSERT_EQ(0, send_result);
    errno = 0;
    receive_result = cp_receive_memory(sockets[1], result);
    FT_ASSERT_EQ(-1, receive_result);
    FT_ASSERT_EQ(EINVAL, errno);
    close_result = ::close(sockets[0]);
    FT_ASSERT_EQ(0, close_result);
    close_result = ::close(sockets[1]);
    FT_ASSERT_EQ(0, close_result);
    index = 0;
    while (index < payload_length)
    {
        FT_ASSERT_EQ(static_cast<unsigned char>(payload[index]), mapping[data_offset + index]);
        index++;
    }
    int stored_error_value;

    std::memcpy(&stored_error_value, mapping + error_offset, sizeof(int));
    FT_ASSERT_EQ(123, stored_error_value);
    FT_ASSERT_EQ(0, pthread_mutex_destroy(reinterpret_cast<pthread_mutex_t *>(mapping)));
    FT_ASSERT_EQ(0, munmap(mapping_ptr, static_cast<size_t>(message.remote_memory_size)));
    FT_ASSERT_EQ(0, shm_unlink(message.shared_memory_name));
    return (1);
}

FT_TEST(test_cross_process_receive_memory_invalid_error_offset, "cross process receive memory invalid error offset")
{
    cross_process_message message;
    cross_process_read_result result;
    void *mapping_ptr;
    unsigned char *mapping;
    size_t data_offset;
    size_t error_offset;
    int sockets[2];
    int send_result;
    int receive_result;
    int close_result;
    const char *payload;
    size_t payload_length;
    size_t index;

    payload = "payload error";
    payload_length = std::strlen(payload);
    mapping_ptr = ft_nullptr;
    mapping = ft_nullptr;
    data_offset = 0;
    error_offset = 0;
    FT_ASSERT_EQ(0, create_shared_memory("/cross_process_invalid_error", payload, payload_length, message, mapping_ptr, mapping,
        data_offset, error_offset));
    message.error_memory_address = message.stack_base_address + message.remote_memory_size;
    FT_ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
    send_result = cp_send_descriptor(sockets[0], message);
    FT_ASSERT_EQ(0, send_result);
    errno = 0;
    receive_result = cp_receive_memory(sockets[1], result);
    FT_ASSERT_EQ(-1, receive_result);
    FT_ASSERT_EQ(EINVAL, errno);
    close_result = ::close(sockets[0]);
    FT_ASSERT_EQ(0, close_result);
    close_result = ::close(sockets[1]);
    FT_ASSERT_EQ(0, close_result);
    index = 0;
    while (index < payload_length)
    {
        FT_ASSERT_EQ(static_cast<unsigned char>(payload[index]), mapping[data_offset + index]);
        index++;
    }
    int error_snapshot;

    std::memcpy(&error_snapshot, mapping + error_offset, sizeof(int));
    FT_ASSERT_EQ(123, error_snapshot);
    FT_ASSERT_EQ(0, pthread_mutex_destroy(reinterpret_cast<pthread_mutex_t *>(mapping)));
    FT_ASSERT_EQ(0, munmap(mapping_ptr, static_cast<size_t>(message.remote_memory_size)));
    FT_ASSERT_EQ(0, shm_unlink(message.shared_memory_name));
    return (1);
}

FT_TEST(test_cross_process_receive_memory_descriptor_disconnect, "cross process receive memory descriptor disconnect")
{
    cross_process_read_result result;
    int sockets[2];
    int close_result;
    int receive_result;

    FT_ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
    FT_ASSERT_EQ(0, ::close(sockets[0]));
    errno = 0;
    receive_result = cp_receive_memory(sockets[1], result);
    FT_ASSERT_EQ(-1, receive_result);
    FT_ASSERT_EQ(ECONNRESET, errno);
    close_result = ::close(sockets[1]);
    FT_ASSERT_EQ(0, close_result);
    return (1);
}

FT_TEST(test_cross_process_receive_memory_missing_mapping, "cross process receive memory missing mapping")
{
    cross_process_message message;
    cross_process_read_result result;
    int sockets[2];
    int send_result;
    int receive_result;
    int close_result;
    char shared_memory_name[64];

    std::memset(&message, 0, sizeof(message));
    std::snprintf(shared_memory_name, sizeof(shared_memory_name), "/cross_process_missing_%ld", static_cast<long>(getpid()));
    if (shm_unlink(shared_memory_name) != 0)
    {
        FT_ASSERT_EQ(ENOENT, errno);
    }
    std::snprintf(message.shared_memory_name, sizeof(message.shared_memory_name), "%s", shared_memory_name);
    message.remote_memory_size = 4096;
    FT_ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
    send_result = cp_send_descriptor(sockets[0], message);
    FT_ASSERT_EQ(0, send_result);
    errno = 0;
    receive_result = cp_receive_memory(sockets[1], result);
    FT_ASSERT_EQ(-1, receive_result);
    FT_ASSERT_EQ(ENOENT, errno);
    close_result = ::close(sockets[0]);
    FT_ASSERT_EQ(0, close_result);
    close_result = ::close(sockets[1]);
    FT_ASSERT_EQ(0, close_result);
    return (1);
}

FT_TEST(test_cross_process_receive_memory_invalid_mutex_offset, "cross process receive memory invalid mutex offset")
{
    cross_process_message message;
    cross_process_read_result result;
    void *mapping_ptr;
    unsigned char *mapping;
    size_t data_offset;
    size_t error_offset;
    int sockets[2];
    int send_result;
    int receive_result;
    int close_result;
    const char *payload;
    size_t payload_length;
    size_t index;
    pthread_mutex_t *shared_mutex;

    payload = "mutex offset payload";
    payload_length = std::strlen(payload);
    mapping_ptr = ft_nullptr;
    mapping = ft_nullptr;
    data_offset = 0;
    error_offset = 0;
    FT_ASSERT_EQ(0, create_shared_memory("/cross_process_invalid_mutex", payload, payload_length, message, mapping_ptr, mapping,
        data_offset, error_offset));
    shared_mutex = reinterpret_cast<pthread_mutex_t *>(mapping);
    message.shared_mutex_address = message.stack_base_address + message.remote_memory_size;
    FT_ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
    send_result = cp_send_descriptor(sockets[0], message);
    FT_ASSERT_EQ(0, send_result);
    errno = 0;
    receive_result = cp_receive_memory(sockets[1], result);
    FT_ASSERT_EQ(-1, receive_result);
    FT_ASSERT_EQ(EINVAL, errno);
    close_result = ::close(sockets[0]);
    FT_ASSERT_EQ(0, close_result);
    close_result = ::close(sockets[1]);
    FT_ASSERT_EQ(0, close_result);
    index = 0;
    while (index < payload_length)
    {
        FT_ASSERT_EQ(static_cast<unsigned char>(payload[index]), mapping[data_offset + index]);
        index++;
    }
    int stored_error_value;

    std::memcpy(&stored_error_value, mapping + error_offset, sizeof(int));
    FT_ASSERT_EQ(123, stored_error_value);
    FT_ASSERT_EQ(0, pthread_mutex_destroy(shared_mutex));
    FT_ASSERT_EQ(0, munmap(mapping_ptr, static_cast<size_t>(message.remote_memory_size)));
    FT_ASSERT_EQ(0, shm_unlink(message.shared_memory_name));
    return (1);
}
#endif
