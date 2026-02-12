#include "compatebility_internal.hpp"
#include <cerrno>
#include <atomic>
#include "../PThread/pthread.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
# include <synchapi.h>

static int32_t cmp_thread_map_windows_error(DWORD error_code)
{
    if (error_code == 0)
        return (FT_ERR_INTERNAL);
    return (cmp_map_system_error_to_ft(static_cast<int32_t>(error_code)));
}

int32_t cmp_thread_equal(pthread_t thread1, pthread_t thread2)
{
    return (thread1 == thread2);
}

int32_t cmp_thread_cancel(pthread_t thread)
{
    if (TerminateThread((HANDLE)thread, 0) == 0)
    {
        return (cmp_thread_map_windows_error(GetLastError()));
    }
    return (FT_ERR_SUCCESS);
}

int32_t cmp_thread_yield()
{
    SwitchToThread();
    return (FT_ERR_SUCCESS);
}

int32_t cmp_thread_sleep(uint32_t milliseconds)
{
    Sleep(milliseconds);
    return (FT_ERR_SUCCESS);
}

int32_t cmp_thread_wait_uint32(std::atomic<uint32_t> *address, uint32_t expected_value)
{
    BOOL wait_result;
    DWORD error_code;

    if (address == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    while (1)
    {
        wait_result = WaitOnAddress(reinterpret_cast<volatile VOID *>(address),
                &expected_value, sizeof(uint32_t), INFINITE);
        if (wait_result != FALSE)
        {
            return (FT_ERR_SUCCESS);
        }
        error_code = GetLastError();
        if (error_code == ERROR_SUCCESS)
        {
            return (FT_ERR_SUCCESS);
        }
        if (error_code == ERROR_TIMEOUT)
            continue;
        return (cmp_thread_map_windows_error(error_code));
    }
}

int32_t cmp_thread_wake_one_uint32(std::atomic<uint32_t> *address)
{
    if (address == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    WakeByAddressSingle(reinterpret_cast<volatile VOID *>(address));
    return (FT_ERR_SUCCESS);
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
    uint32_t waiter_count;
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

static int32_t cmp_wait_create_entry(std::atomic<uint32_t> *address, cmp_wait_entry **entry_out) noexcept
{
    cmp_wait_entry *new_entry;
    int32_t init_result;

    if (!entry_out)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    new_entry = reinterpret_cast<cmp_wait_entry *>(cma_malloc(sizeof(cmp_wait_entry)));
    if (!new_entry)
    {
        return (FT_ERR_NO_MEMORY);
    }
    new_entry->address = address;
    new_entry->waiter_count = 0;
    new_entry->next = g_wait_list_head;
    init_result = pthread_cond_init(&new_entry->condition, ft_nullptr);
    if (init_result != 0)
    {
        cma_free(new_entry);
        return (cmp_map_system_error_to_ft(init_result));
    }
    g_wait_list_head = new_entry;
    *entry_out = new_entry;
    return (FT_ERR_SUCCESS);
}

static int32_t cmp_wait_remove_entry(cmp_wait_entry *entry) noexcept
{
    cmp_wait_entry *current_entry;
    cmp_wait_entry *previous_entry;
    int32_t destroy_result;

    if (!entry)
    {
        return (FT_ERR_INVALID_ARGUMENT);
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
                return (cmp_map_system_error_to_ft(destroy_result));
            }
            if (previous_entry == ft_nullptr)
                g_wait_list_head = current_entry->next;
            else
                previous_entry->next = current_entry->next;
            cma_free(current_entry);
            return (FT_ERR_SUCCESS);
        }
        previous_entry = current_entry;
        current_entry = current_entry->next;
    }
    return (FT_ERR_SUCCESS);
}
# endif

int32_t cmp_thread_equal(pthread_t thread1, pthread_t thread2)
{
    return (pthread_equal(thread1, thread2));
}

int32_t cmp_thread_cancel(pthread_t thread)
{
    int32_t return_value;

    return_value = pthread_cancel(thread);
    if (return_value != 0)
    {
        return (cmp_map_system_error_to_ft(return_value));
    }
    return (FT_ERR_SUCCESS);
}

int32_t cmp_thread_yield()
{
    if (sched_yield() != 0)
    {
        return (cmp_map_system_error_to_ft(errno));
    }
    return (FT_ERR_SUCCESS);
}

int32_t cmp_thread_sleep(uint32_t milliseconds)
{
    if (usleep(milliseconds * 1000) == -1)
    {
        return (cmp_map_system_error_to_ft(errno));
    }
    return (FT_ERR_SUCCESS);
}

int32_t cmp_thread_wait_uint32(std::atomic<uint32_t> *address, uint32_t expected_value)
{
# if defined(__linux__)
    int64_t syscall_result;

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
            return (FT_ERR_SUCCESS);
        }
        if (errno == EAGAIN)
        {
            return (FT_ERR_SUCCESS);
        }
        if (errno == EINTR)
            continue;
        return (cmp_map_system_error_to_ft(errno));
    }
# else
    int32_t lock_error;
    cmp_wait_entry *entry;
    int32_t create_error;
    int32_t wait_result;
    int32_t remove_error;
    int32_t unlock_error;
    uint32_t current_value;

    if (address == ft_nullptr)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    lock_error = pt_pthread_mutex_lock_with_error(&g_wait_list_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (lock_error);
    }
    entry = cmp_wait_lookup_entry(address);
    if (entry == ft_nullptr)
    {
        create_error = cmp_wait_create_entry(address, &entry);
        if (create_error != FT_ERR_SUCCESS)
        {
            unlock_error = pt_pthread_mutex_unlock_with_error(&g_wait_list_mutex);
            if (unlock_error != FT_ERR_SUCCESS)
                return (unlock_error);
            return (create_error);
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
            if (entry->waiter_count > 0)
                entry->waiter_count -= 1;
            if (entry->waiter_count == 0)
            {
                remove_error = cmp_wait_remove_entry(entry);
                if (remove_error != FT_ERR_SUCCESS)
                {
                    unlock_error = pt_pthread_mutex_unlock_with_error(&g_wait_list_mutex);
                    if (unlock_error != FT_ERR_SUCCESS)
                        return (unlock_error);
                    return (remove_error);
                }
            }
            unlock_error = pt_pthread_mutex_unlock_with_error(&g_wait_list_mutex);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                return (unlock_error);
            }
            return (cmp_map_system_error_to_ft(wait_result));
        }
    }
    if (entry->waiter_count > 0)
        entry->waiter_count -= 1;
    if (entry->waiter_count == 0)
    {
        remove_error = cmp_wait_remove_entry(entry);
        if (remove_error != FT_ERR_SUCCESS)
        {
            unlock_error = pt_pthread_mutex_unlock_with_error(&g_wait_list_mutex);
            if (unlock_error != FT_ERR_SUCCESS)
                return (unlock_error);
            return (remove_error);
        }
    }
    unlock_error = pt_pthread_mutex_unlock_with_error(&g_wait_list_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
# endif
}

int32_t cmp_thread_wake_one_uint32(std::atomic<uint32_t> *address)
{
# if defined(__linux__)
    int64_t syscall_result;

    syscall_result = syscall(SYS_futex, reinterpret_cast<uint32_t *>(address),
#  ifdef FUTEX_WAKE_PRIVATE
            FUTEX_WAKE_PRIVATE,
#  else
            FUTEX_WAKE,
#  endif
            1, NULL, NULL, 0);
    if (syscall_result == -1)
    {
        return (cmp_map_system_error_to_ft(errno));
    }
    return (FT_ERR_SUCCESS);
# else
    int32_t lock_error;
    cmp_wait_entry *entry;
    int32_t signal_result;
    int32_t unlock_error;

    if (address == ft_nullptr)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    lock_error = pt_pthread_mutex_lock_with_error(&g_wait_list_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        return (lock_error);
    }
    entry = cmp_wait_lookup_entry(address);
    if (entry != ft_nullptr)
    {
        signal_result = pthread_cond_signal(&entry->condition);
        if (signal_result != 0)
        {
            unlock_error = pt_pthread_mutex_unlock_with_error(&g_wait_list_mutex);
            if (unlock_error != FT_ERR_SUCCESS)
                return (unlock_error);
            return (cmp_map_system_error_to_ft(signal_result));
        }
    }
    unlock_error = pt_pthread_mutex_unlock_with_error(&g_wait_list_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
# endif
}
#endif
