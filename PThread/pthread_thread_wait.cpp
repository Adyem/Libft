#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <atomic>

int pt_thread_wait_uint32(std::atomic<uint32_t> *address, uint32_t expected_value)
{
    return (cmp_thread_wait_uint32(address, expected_value));
}

int pt_thread_wake_one_uint32(std::atomic<uint32_t> *address)
{
    return (cmp_thread_wake_one_uint32(address));
}

