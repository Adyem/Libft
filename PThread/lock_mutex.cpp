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
    if (_owner.load(std::memory_order_relaxed) == thread_id)
    {
        ft_errno = PT_ERR_ALRDY_LOCKED;
        set_error(PT_ERR_ALRDY_LOCKED);
        return (-1);
    }
    set_error(ER_SUCCESS);
    const uint32_t my = _next.fetch_add(1, std::memory_order_acq_rel);
    uint32_t spins = 0;
    for (;;)
    {
        uint32_t cur = _serving.load(std::memory_order_acquire);
        if (cur == my)
            break;
        if (++spins < 64)
            cpu_relax();
        else
            pt_thread_yield();
    }
    _owner.store(thread_id, std::memory_order_relaxed);
    _lock = true;
    return (SUCCES);
}

