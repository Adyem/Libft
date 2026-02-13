#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <atomic>
#include <errno.h>

int pt_thread_wait_uint32(std::atomic<uint32_t> *address, uint32_t expected_value)
{
    int return_value;

    return_value = cmp_thread_wait_uint32(address, expected_value);
    if (return_value != 0)
        return (return_value);
    return (return_value);
}

int pt_thread_wake_one_uint32(std::atomic<uint32_t> *address)
{
    int return_value;

    return_value = cmp_thread_wake_one_uint32(address);
    if (return_value != 0)
        return (return_value);
    return (return_value);
}
