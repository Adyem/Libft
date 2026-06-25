#include "printf_internal.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <climits>
#include <new>
#include "../PThread/mutex.hpp"

static pt_recursive_mutex *g_pf_custom_specifiers_mutex = ft_nullptr;

int32_t pf_enable_thread_safety(void)
{
    pt_recursive_mutex *mutex_pointer;
    int32_t mutex_error;

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

int32_t pf_disable_thread_safety(void)
{
    int32_t destroy_error;

    if (g_pf_custom_specifiers_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = g_pf_custom_specifiers_mutex->destroy();
    delete g_pf_custom_specifiers_mutex;
    g_pf_custom_specifiers_mutex = ft_nullptr;
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
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

int32_t pf_register_custom_specifier(char specifier, t_pf_custom_formatter handler, void *context)
{
    uint8_t               index;
    t_pf_custom_specifier_entry *entry;
    int32_t                     lock_error;
    int32_t                     result;

    lock_error = pt_recursive_mutex_lock_if_not_null(g_pf_custom_specifiers_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    result = FT_ERR_SUCCESS;
    if (handler == ft_nullptr)
    {
        result = FT_ERR_INVALID_POINTER;
        goto unlock;
    }
    index = static_cast<uint8_t>(specifier);
    entry = &g_pf_custom_specifiers[index];
    if (entry->handler != ft_nullptr)
    {
        result = FT_ERR_ALREADY_EXISTS;
        goto unlock;
    }
    entry->handler = handler;
    entry->context = context;
unlock:
    (void)pt_recursive_mutex_unlock_if_not_null(g_pf_custom_specifiers_mutex);
    return (result);
}

int32_t pf_unregister_custom_specifier(char specifier)
{
    uint8_t index;
    int32_t       lock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(g_pf_custom_specifiers_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    index = static_cast<uint8_t>(specifier);
    pf_clear_entry(g_pf_custom_specifiers[index]);
    (void)pt_recursive_mutex_unlock_if_not_null(g_pf_custom_specifiers_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t pf_try_format_custom_specifier(char specifier, va_list *argument_list, ft_string &output, ft_bool *handled)
{
    uint8_t         index;
    t_pf_custom_formatter handler;
    void                  *context;
    int32_t                   callback_error;
    int32_t                   lock_error;
    int32_t               initialization_error;
    int32_t               clear_error;

    if (handled == ft_nullptr)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    *handled = FT_FALSE;
    if (argument_list == ft_nullptr)
    {
        return (FT_ERR_INVALID_POINTER);
    }
    initialization_error = output.initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    index = static_cast<uint8_t>(specifier);
    lock_error = pt_recursive_mutex_lock_if_not_null(g_pf_custom_specifiers_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    handler = g_pf_custom_specifiers[index].handler;
    context = g_pf_custom_specifiers[index].context;
    (void)pt_recursive_mutex_unlock_if_not_null(g_pf_custom_specifiers_mutex);
    if (handler == ft_nullptr)
        return (FT_ERR_SUCCESS);
    clear_error = output.clear();
    if (clear_error != FT_ERR_SUCCESS)
        return (clear_error);
    *handled = FT_TRUE;
    callback_error = handler(argument_list, output, context);
    if (callback_error != FT_ERR_SUCCESS)
        return (callback_error);
    return (FT_ERR_SUCCESS);
}
