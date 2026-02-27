#include "printf_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <climits>
#include <new>

static pt_recursive_mutex *g_pf_custom_specifiers_mutex = ft_nullptr;

static int pf_custom_specifiers_lock(void)
{
    return (pt_recursive_mutex_lock_if_not_null(g_pf_custom_specifiers_mutex));
}

static int pf_custom_specifiers_unlock(void)
{
    return (pt_recursive_mutex_unlock_if_not_null(g_pf_custom_specifiers_mutex));
}

int pf_enable_thread_safety(void)
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    if (g_pf_custom_specifiers_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    g_pf_custom_specifiers_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int pf_disable_thread_safety(void)
{
    int destroy_error;

    if (g_pf_custom_specifiers_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = g_pf_custom_specifiers_mutex->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    delete g_pf_custom_specifiers_mutex;
    g_pf_custom_specifiers_mutex = ft_nullptr;
    return (FT_ERR_SUCCESS);
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
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (handler == ft_nullptr)
        goto unlock;
    index = static_cast<unsigned char>(specifier);
    entry = &g_pf_custom_specifiers[index];
    if (entry->handler != ft_nullptr)
        goto unlock;
    entry->handler = handler;
    entry->context = context;
    result = 0;
unlock:
    unlock_error = pf_custom_specifiers_unlock();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (result);
}

int pf_unregister_custom_specifier(char specifier)
{
    unsigned char index;
    int           lock_error;
    int           unlock_error;

    lock_error = pf_custom_specifiers_lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    index = static_cast<unsigned char>(specifier);
    pf_clear_entry(g_pf_custom_specifiers[index]);
    unlock_error = pf_custom_specifiers_unlock();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int pf_try_format_custom_specifier(char specifier, va_list *argument_list, ft_string &output, bool *handled)
{
    unsigned char         index;
    t_pf_custom_formatter handler;
    void                  *context;
    int                   error_code;
    int                   lock_error;
    int                   unlock_error;
    int32_t               initialization_error;

    if (handled == ft_nullptr)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    *handled = false;
    if (argument_list == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    initialization_error = output.initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    index = static_cast<unsigned char>(specifier);
    lock_error = pf_custom_specifiers_lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    handler = g_pf_custom_specifiers[index].handler;
    context = g_pf_custom_specifiers[index].context;
    unlock_error = pf_custom_specifiers_unlock();
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    if (handler == ft_nullptr)
        return (FT_ERR_SUCCESS);
    output.clear();
    int string_error = pf_string_pop_last_error(output);
    if (string_error != FT_ERR_SUCCESS)
        return (string_error);
    *handled = true;
    if (handler(argument_list, output, context) != 0)
    {
        string_error = pf_string_pop_last_error(output);
        error_code = string_error;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INTERNAL;
        return (error_code);
    }
    string_error = pf_string_pop_last_error(output);
    if (string_error != FT_ERR_SUCCESS)
        return (string_error);
    return (FT_ERR_SUCCESS);
}
