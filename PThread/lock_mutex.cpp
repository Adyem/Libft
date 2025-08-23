#include "PThread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"

#undef FAILURE
#define FAILURE -1
#undef SUCCES
#define SUCCES 0

int pt_mutex::lock(pthread_t thread_id)
{
    (void)thread_id;
    ft_errno = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
    size_t spins = 0;
    while (this->_flag.test_and_set(std::memory_order_acquire))
    {
        if (++spins < 100)
        {
#if defined(__x86_64__) || defined(__i386) || defined(_M_X64) || defined(_M_IX86)
            __asm__ __volatile__("pause");
#endif
        }
        else
        {
            pt_thread_yield();
            spins = 0;
        }
    }
    this->_lock = true;
    return (SUCCES);
}
