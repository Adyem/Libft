#include "config.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include <cstring>
#include <cctype>

char *cnfg_parse_flags(int argument_count, char **argument_values)
{
    if (argument_count <= 1 || !argument_values)
        return (ft_nullptr);
    char   *flags = ft_nullptr;
    size_t  length = 0;
    int argument_index = 1;
    while (argument_index < argument_count)
    {
        char *argument = argument_values[argument_index];
        if (!argument || argument[0] != '-' || argument[1] == '-' || argument[1] == '\0')
        {
            ++argument_index;
            continue ;
        }
        int char_index = 1;
        while (argument[char_index])
        {
            char current_flag = argument[char_index];
            if (std::isalpha(static_cast<unsigned char>(current_flag)))
            {
                if (!flags || !std::strchr(flags, current_flag))
                {
                    char *new_flags = static_cast<char*>(cma_realloc(flags, length + 2));
                    if (!new_flags)
                    {
                        ft_errno = FT_EALLOC;
                        cma_free(flags);
                        return (ft_nullptr);
                    }
                    flags = new_flags;
                    flags[length++] = current_flag;
                    flags[length] = '\0';
                }
            }
            ++char_index;
        }
        ++argument_index;
    }
    return (flags);
}

char **cnfg_parse_long_flags(int argument_count, char **argument_values)
{
    if (argument_count <= 1 || !argument_values)
        return (ft_nullptr);
    char  **flags = ft_nullptr;
    size_t  count = 0;
    int argument_index = 1;
    while (argument_index < argument_count)
    {
        char *argument = argument_values[argument_index];
        if (!argument || argument[0] != '-' || argument[1] != '-' || argument[2] == '\0')
        {
            ++argument_index;
            continue ;
        }
        const char *flag_string = argument + 2;
        bool exists = false;
        size_t flag_index = 0;
        while (flag_index < count)
        {
            if (std::strcmp(flags[flag_index], flag_string) == 0)
            {
                exists = true;
                break ;
            }
            ++flag_index;
        }
        if (exists)
        {
            ++argument_index;
            continue ;
        }
        char **new_flags = static_cast<char**>(cma_realloc(flags, (count + 2) * sizeof(char*)));
        if (!new_flags)
        {
            ft_errno = FT_EALLOC;
            size_t free_index = 0;
            while (free_index < count)
            {
                cma_free(flags[free_index]);
                ++free_index;
            }
            cma_free(flags);
            return (ft_nullptr);
        }
        flags = new_flags;
        flags[count] = cma_strdup(flag_string);
        if (!flags[count])
        {
            ft_errno = FT_EALLOC;
            size_t free_index = 0;
            while (free_index < count)
            {
                cma_free(flags[free_index]);
                ++free_index;
            }
            cma_free(flags);
            return (ft_nullptr);
        }
        ++count;
        flags[count] = ft_nullptr;
        ++argument_index;
    }
    return (flags);
}

