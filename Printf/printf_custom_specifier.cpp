#include "printf_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"
#include <climits>
#include <cstdlib>
#include <pthread.h>

static pthread_mutex_t *g_pf_custom_specifiers_mutex = ft_nullptr;

static int pf_custom_specifiers_lock(void)
{
    if (g_pf_custom_specifiers_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int mutex_error = pt_pthread_mutex_lock_with_error(g_pf_custom_specifiers_mutex);
    int stack_error = ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(stack_error);
        return (mutex_error);
    }
    return (FT_ERR_SUCCESSS);
}

static int pf_custom_specifiers_unlock(void)
{
    if (g_pf_custom_specifiers_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int mutex_error = pt_pthread_mutex_unlock_with_error(g_pf_custom_specifiers_mutex);
    int stack_error = ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(stack_error);
        return (mutex_error);
    }
    return (FT_ERR_SUCCESSS);
}

int pf_enable_thread_safety(void)
{
    if (g_pf_custom_specifiers_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    pthread_mutex_t *mutex_pointer = static_cast<pthread_mutex_t*>(std::malloc(sizeof(pthread_mutex_t)));
    if (mutex_pointer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    int pthread_error = pthread_mutex_init(mutex_pointer, ft_nullptr);
    if (pthread_error != 0)
    {
        int error_code = ft_map_system_error(pthread_error);
        std::free(mutex_pointer);
        ft_global_error_stack_push(error_code);
        return (error_code);
    }
    g_pf_custom_specifiers_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

void pf_disable_thread_safety(void)
{
    if (g_pf_custom_specifiers_mutex == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    int pthread_error = pthread_mutex_destroy(g_pf_custom_specifiers_mutex);
    std::free(g_pf_custom_specifiers_mutex);
    g_pf_custom_specifiers_mutex = ft_nullptr;
    if (pthread_error != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(pthread_error));
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

typedef struct s_pf_custom_specifier_entry
{
    t_pf_custom_formatter   handler;
    void                    *context;
}   t_pf_custom_specifier_entry;

static t_pf_custom_specifier_entry  g_pf_custom_specifiers[UCHAR_MAX + 1];

static void pf_clear_entry(t_pf_custom_specifier_entry &entry)
{
    entry.handler = static_cast<t_pf_custom_formatter>(ft_nullptr);
    entry.context = ft_nullptr;
    return ;
}

int pf_register_custom_specifier(char specifier, t_pf_custom_formatter handler, void *context)
{
    unsigned char               index;
    t_pf_custom_specifier_entry *entry;
    int                         lock_error;
    int                         unlock_error;
    int                         result = -1;

    lock_error = pf_custom_specifiers_lock();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_LOCK_FAILED);
        return (-1);
    }
    if (handler == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_POINTER);
        goto unlock;
    }
    index = static_cast<unsigned char>(specifier);
    entry = &g_pf_custom_specifiers[index];
    if (entry->handler != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_ALREADY_EXISTS);
        goto unlock;
    }
    entry->handler = handler;
    entry->context = context;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    result = 0;
unlock:
    unlock_error = pf_custom_specifiers_unlock();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
        return (-1);
    }
    return (result);
}

int pf_unregister_custom_specifier(char specifier)
{
    unsigned char index;
    int           lock_error;
    int           unlock_error;

    lock_error = pf_custom_specifiers_lock();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_LOCK_FAILED);
        return (-1);
    }
    index = static_cast<unsigned char>(specifier);
    pf_clear_entry(g_pf_custom_specifiers[index]);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    unlock_error = pf_custom_specifiers_unlock();
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
        return (-1);
    }
    return (0);
}

int pf_try_format_custom_specifier(char specifier, va_list *args, ft_string &output, bool *handled)
{
    unsigned char         index;
    t_pf_custom_formatter handler;
    void                  *context;
    int                   error_code;
    int                   lock_error;
    int                   unlock_error;

    if (handled == ft_nullptr)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    *handled = false;
    if (args == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    index = static_cast<unsigned char>(specifier);
    lock_error = pf_custom_specifiers_lock();
    if (lock_error != FT_ERR_SUCCESSS)
        return (lock_error);
    handler = g_pf_custom_specifiers[index].handler;
    context = g_pf_custom_specifiers[index].context;
    unlock_error = pf_custom_specifiers_unlock();
    if (unlock_error != FT_ERR_SUCCESSS)
        return (unlock_error);
    if (handler == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    output.clear();
    int string_error = pf_string_pop_last_error(output);
    if (string_error != FT_ERR_SUCCESSS)
        return (string_error);
    *handled = true;
    if (handler(args, output, context) != 0)
    {
        string_error = pf_string_pop_last_error(output);
        error_code = string_error;
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
        return (error_code);
    }
    string_error = pf_string_pop_last_error(output);
    if (string_error != FT_ERR_SUCCESSS)
        return (string_error);
    return (FT_ERR_SUCCESSS);
}
