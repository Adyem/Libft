#include "CMA.hpp"
#include "../CPP_class/cpp_class_nullptr.hpp"

static int    ft_count_words(const char *string, char delimiter)
{
    int    word_count;
    int    index;

    word_count = 0;
    index = 0;
    while (string[index])
    {
        if (index == 0 && string[index] != delimiter)
            word_count++;
        if (index > 0 && string[index] != delimiter && string[index - 1] == delimiter)
            word_count++;
        index++;
    }
    return (word_count);
}

static char    **ft_malloc_strings(char **strings, const char *string, char delimiter)
{
    int    char_count;
    int    index;
    int    array_index;

    char_count = 0;
    index = 0;
    array_index = 0;
    while (string[index])
    {
        if (string[index] != delimiter)
            char_count++;
        if ((string[index] == delimiter && index > 0 && string[index - 1] != delimiter)
            || (string[index] != delimiter && string[index + 1] == '\0'))
        {
            strings[array_index] = static_cast<char *>(cma_malloc(sizeof(char) * (char_count + 1)));
            if (!strings[array_index])
                return (ft_nullptr);
            char_count = 0;
            array_index++;
        }
        index++;
    }
    return (strings);
}

static char    **ft_copy_strings(char **strings, const char *string, char delimiter)
{
    int    index;
    int    array_index;
    int    string_index;

    index = 0;
    array_index = 0;
    string_index = 0;
    while (string[index])
    {
        if (string[index] != delimiter)
            strings[array_index][string_index++] = string[index];
        if (string[index] != delimiter && string[index + 1] == '\0')
            strings[array_index][string_index] = '\0';
        if (string[index] == delimiter && index > 0 && string[index - 1] != delimiter)
        {
            strings[array_index][string_index] = '\0';
            array_index++;
            string_index = 0;
        }
        index++;
    }
    return (strings);
}

static char    **ft_memory_error(char **strings)
{
    int    index;

    index = 0;
    while (strings[index])
    {
        cma_free(strings[index]);
        strings[index] = ft_nullptr;
        index++;
    }
    cma_free(strings);
    return (ft_nullptr);
}

char    **cma_split(char const *string, char delimiter)
{
    char    **strings;
    int        word_count;

    if (!string)
    {
        strings = static_cast<char **>(cma_malloc(sizeof(char) * 1));
        if (!strings)
            return (ft_nullptr);
        *strings = ft_nullptr;
        return (strings);
    }
    word_count = ft_count_words(string, delimiter);
    strings = static_cast<char **>(cma_malloc(sizeof(*strings) * (word_count + 1)));
    if (!strings)
        return (ft_nullptr);
    if (ft_malloc_strings(strings, string, delimiter))
    {
        ft_copy_strings(strings, string, delimiter);
        strings[word_count] = ft_nullptr;
    }
    else
        strings = ft_memory_error(strings);
    return (strings);
}
