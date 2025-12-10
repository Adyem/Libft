#include "compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <cerrno>
#include <atomic>

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
# include <synchapi.h>
int cmp_thread_equal(pthread_t thread1, pthread_t thread2)
{
    return (thread1 == thread2);
}

int cmp_thread_cancel(pthread_t thread)
{
    if (TerminateThread((HANDLE)thread, 0) == 0)
    {
        ft_errno = ft_map_system_error(GetLastError());
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int cmp_thread_yield()
{
    SwitchToThread();
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int cmp_thread_sleep(unsigned int milliseconds)
{
    Sleep(milliseconds);
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int cmp_thread_wait_uint32(std::atomic<uint32_t> *address, uint32_t expected_value)
{
    BOOL wait_result;
    DWORD error_code;

    while (1)
    {
        wait_result = WaitOnAddress(reinterpret_cast<volatile VOID *>(address),
                &expected_value, sizeof(uint32_t), INFINITE);
        if (wait_result != FALSE)
        {
            ft_errno = FT_ER_SUCCESSS;
            return (0);
        }
        error_code = GetLastError();
        if (error_code == ERROR_SUCCESS)
        {
            ft_errno = FT_ER_SUCCESSS;
            return (0);
        }
        if (error_code == ERROR_TIMEOUT)
            continue;
        ft_errno = ft_map_system_error(error_code);
        return (-1);
    }
}

int cmp_thread_wake_one_uint32(std::atomic<uint32_t> *address)
{
    WakeByAddressSingle(reinterpret_cast<volatile VOID *>(address));
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}
#else
# include <sched.h>
# include <unistd.h>
# include <errno.h>
# if defined(__linux__)
#  include <linux/futex.h>
#  include <sys/syscall.h>
# else
#  include "../CMA/CMA.hpp"
#  include "../CPP_class/class_nullptr.hpp"

struct cmp_wait_entry
{
    std::atomic<uint32_t> *address;
    pthread_cond_t condition;
    unsigned int waiter_count;
    cmp_wait_entry *next;
};

static pthread_mutex_t g_wait_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static cmp_wait_entry *g_wait_list_head = ft_nullptr;

static cmp_wait_entry *cmp_wait_lookup_entry(std::atomic<uint32_t> *address) noexcept
{
    cmp_wait_entry *current_entry;

    current_entry = g_wait_list_head;
    while (current_entry != ft_nullptr)
    {
        if (current_entry->address == address)
            return (current_entry);
        current_entry = current_entry->next;
    }
    return (ft_nullptr);
}

static int cmp_wait_create_entry(std::atomic<uint32_t> *address, cmp_wait_entry **entry_out) noexcept
{
    cmp_wait_entry *new_entry;
    int init_result;

    if (!entry_out)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    new_entry = reinterpret_cast<cmp_wait_entry *>(cma_malloc(sizeof(cmp_wait_entry)));
    if (!new_entry)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    new_entry->address = address;
    new_entry->waiter_count = 0;
    new_entry->next = g_wait_list_head;
    init_result = pthread_cond_init(&new_entry->condition, ft_nullptr);
    if (init_result != 0)
    {
        cma_free(new_entry);
        ft_errno = ft_map_system_error(init_result);
        return (-1);
    }
    g_wait_list_head = new_entry;
    *entry_out = new_entry;
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

static int cmp_wait_remove_entry(cmp_wait_entry *entry) noexcept
{
    cmp_wait_entry *current_entry;
    cmp_wait_entry *previous_entry;
    int destroy_result;

    if (!entry)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    previous_entry = ft_nullptr;
    current_entry = g_wait_list_head;
    while (current_entry != ft_nullptr)
    {
        if (current_entry == entry)
        {
            destroy_result = pthread_cond_destroy(&entry->condition);
            if (destroy_result != 0)
            {
                ft_errno = ft_map_system_error(destroy_result);
                return (-1);
            }
            if (previous_entry == ft_nullptr)
                g_wait_list_head = current_entry->next;
            else
                previous_entry->next = current_entry->next;
            cma_free(current_entry);
            ft_errno = FT_ER_SUCCESSS;
            return (0);
        }
        previous_entry = current_entry;
        current_entry = current_entry->next;
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}
# endif

int cmp_thread_equal(pthread_t thread1, pthread_t thread2)
{
    return (pthread_equal(thread1, thread2));
}

int cmp_thread_cancel(pthread_t thread)
{
    int return_value;

    return_value = pthread_cancel(thread);
    if (return_value != 0)
    {
        ft_errno = ft_map_system_error(return_value);
        return (return_value);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (return_value);
}

int cmp_thread_yield()
{
    if (sched_yield() != 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int cmp_thread_sleep(unsigned int milliseconds)
{
    if (usleep(milliseconds * 1000) == -1)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int cmp_thread_wait_uint32(std::atomic<uint32_t> *address, uint32_t expected_value)
{
# if defined(__linux__)
    long syscall_result;

    while (1)
    {
        syscall_result = syscall(SYS_futex, reinterpret_cast<uint32_t *>(address),
#  ifdef FUTEX_WAIT_PRIVATE
                FUTEX_WAIT_PRIVATE,
#  else
                FUTEX_WAIT,
#  endif
                expected_value, NULL, NULL, 0);
        if (syscall_result == 0)
        {
            ft_errno = FT_ER_SUCCESSS;
            return (0);
        }
        if (errno == EAGAIN)
        {
            ft_errno = FT_ER_SUCCESSS;
            return (0);
        }
        if (errno == EINTR)
            continue;
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
# else
    int lock_result;
    cmp_wait_entry *entry;
    int wait_result;
    int unlock_result;
    uint32_t current_value;

    if (address == ft_nullptr)
    {
        ft_errno = ft_map_system_error(EFAULT);
        return (-1);
    }
    lock_result = pthread_mutex_lock(&g_wait_list_mutex);
    if (lock_result != 0)
    {
        ft_errno = ft_map_system_error(lock_result);
        return (-1);
    }
    entry = cmp_wait_lookup_entry(address);
    if (entry == ft_nullptr)
    {
        if (cmp_wait_create_entry(address, &entry) != 0)
        {
            unlock_result = pthread_mutex_unlock(&g_wait_list_mutex);
            if (unlock_result != 0)
                ft_errno = ft_map_system_error(unlock_result);
            return (-1);
        }
    }
    entry->waiter_count += 1;
    while (1)
    {
        current_value = address->load();
        if (current_value != expected_value)
            break ;
        wait_result = pthread_cond_wait(&entry->condition, &g_wait_list_mutex);
        if (wait_result != 0)
        {
            int mapped_error;

            if (entry->waiter_count > 0)
                entry->waiter_count -= 1;
            if (entry->waiter_count == 0)
            {
                if (cmp_wait_remove_entry(entry) != 0)
                {
                    unlock_result = pthread_mutex_unlock(&g_wait_list_mutex);
                    if (unlock_result != 0)
                        ft_errno = ft_map_system_error(unlock_result);
                    return (-1);
                }
            }
            mapped_error = ft_map_system_error(wait_result);
            unlock_result = pthread_mutex_unlock(&g_wait_list_mutex);
            if (unlock_result != 0)
            {
                ft_errno = ft_map_system_error(unlock_result);
                return (-1);
            }
            ft_errno = mapped_error;
            return (-1);
        }
    }
    if (entry->waiter_count > 0)
        entry->waiter_count -= 1;
    if (entry->waiter_count == 0)
    {
        if (cmp_wait_remove_entry(entry) != 0)
        {
            unlock_result = pthread_mutex_unlock(&g_wait_list_mutex);
            if (unlock_result != 0)
                ft_errno = ft_map_system_error(unlock_result);
            return (-1);
        }
    }
    unlock_result = pthread_mutex_unlock(&g_wait_list_mutex);
    if (unlock_result != 0)
    {
        ft_errno = ft_map_system_error(unlock_result);
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
# endif
}

int cmp_thread_wake_one_uint32(std::atomic<uint32_t> *address)
{
# if defined(__linux__)
    long syscall_result;

    syscall_result = syscall(SYS_futex, reinterpret_cast<uint32_t *>(address),
#  ifdef FUTEX_WAKE_PRIVATE
            FUTEX_WAKE_PRIVATE,
#  else
            FUTEX_WAKE,
#  endif
            1, NULL, NULL, 0);
    if (syscall_result == -1)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
# else
    int lock_result;
    cmp_wait_entry *entry;
    int signal_result;
    int unlock_result;

    if (address == ft_nullptr)
    {
        ft_errno = ft_map_system_error(EFAULT);
        return (-1);
    }
    lock_result = pthread_mutex_lock(&g_wait_list_mutex);
    if (lock_result != 0)
    {
        ft_errno = ft_map_system_error(lock_result);
        return (-1);
    }
    entry = cmp_wait_lookup_entry(address);
    if (entry != ft_nullptr)
    {
        signal_result = pthread_cond_signal(&entry->condition);
        if (signal_result != 0)
        {
            unlock_result = pthread_mutex_unlock(&g_wait_list_mutex);
            if (unlock_result != 0)
                ft_errno = ft_map_system_error(unlock_result);
            ft_errno = ft_map_system_error(signal_result);
            return (-1);
        }
    }
    unlock_result = pthread_mutex_unlock(&g_wait_list_mutex);
    if (unlock_result != 0)
    {
        ft_errno = ft_map_system_error(unlock_result);
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
# endif
}
#endif
