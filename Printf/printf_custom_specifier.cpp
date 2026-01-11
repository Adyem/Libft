#include "printf_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <climits>
#include <mutex>

typedef struct s_pf_custom_specifier_entry
{
    t_pf_custom_formatter   handler;
    void                    *context;
}   t_pf_custom_specifier_entry;

static t_pf_custom_specifier_entry  g_pf_custom_specifiers[UCHAR_MAX + 1];
static std::mutex                   g_pf_custom_specifiers_mutex;

static void pf_clear_entry(t_pf_custom_specifier_entry &entry)
{
    entry.handler = static_cast<t_pf_custom_formatter>(ft_nullptr);
    entry.context = ft_nullptr;
    return ;
}

int pf_register_custom_specifier(char specifier, t_pf_custom_formatter handler, void *context)
{
    unsigned char                       index;
    std::lock_guard<std::mutex>         guard(g_pf_custom_specifiers_mutex);
    t_pf_custom_specifier_entry         *entry;

    if (handler == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_POINTER);
        return (-1);
    }
    index = static_cast<unsigned char>(specifier);
    entry = &g_pf_custom_specifiers[index];
    if (entry->handler != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_ALREADY_EXISTS);
        return (-1);
    }
    entry->handler = handler;
    entry->context = context;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int pf_unregister_custom_specifier(char specifier)
{
    unsigned char               index;
    std::lock_guard<std::mutex> guard(g_pf_custom_specifiers_mutex);

    index = static_cast<unsigned char>(specifier);
    pf_clear_entry(g_pf_custom_specifiers[index]);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int pf_try_format_custom_specifier(char specifier, va_list *args, ft_string &output)
{
    unsigned char               index;
    t_pf_custom_formatter       handler;
    void                        *context;

    if (args == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        return (-1);
    }
    index = static_cast<unsigned char>(specifier);
    {
        std::lock_guard<std::mutex> guard(g_pf_custom_specifiers_mutex);
        handler = g_pf_custom_specifiers[index].handler;
        context = g_pf_custom_specifiers[index].context;
    }
    if (handler == ft_nullptr)
        return (1);
    output.clear();
    ft_errno = FT_ERR_SUCCESSS;
    if (handler(args, output, context) != 0)
    {
        if (ft_errno == FT_ERR_SUCCESSS)
            ft_errno = FT_ERR_INTERNAL;
        return (-1);
    }
    int output_error = output.get_error();
    if (output_error != FT_ERR_SUCCESSS)
    {
        if (ft_errno == FT_ERR_SUCCESSS)
            ft_errno = output_error;
        return (-1);
    }
    return (0);
}
