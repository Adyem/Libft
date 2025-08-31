#include "PThread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

static inline void cpu_relax()
{
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
    __builtin_ia32_pause();
#else
    asm volatile("" ::: "memory");
#endif
}

int pt_mutex::lock(pthread_t thread_id)
{
    if (this->_owner.load(std::memory_order_relaxed) == thread_id)
    {
        ft_errno = PT_ERR_ALRDY_LOCKED;
        this->set_error(PT_ERR_ALRDY_LOCKED);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    const uint32_t my = this->_next.fetch_add(1, std::memory_order_acq_rel);
    uint32_t spins = 0;
    while (1)
    {
        uint32_t cur = this->_serving.load(std::memory_order_acquire);
        if (cur == my)
            break;
        if (++spins < 64)
            cpu_relax();
        else
            pt_thread_yield();
    }
    this->_owner.store(thread_id, std::memory_order_relaxed);
    this->_lock = true;
    return (SUCCES);
}
