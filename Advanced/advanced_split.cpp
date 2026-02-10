#include <cstddef>
#include "advanced.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"

static ft_size_t count_words(const char *string, char delimiter)
{
    ft_size_t count = 0;
    bool in_word = false;
    for (ft_size_t index = 0; string[index] != '\0'; ++index)
    {
        if (string[index] != delimiter && !in_word)
        {
            in_word = true;
            ++count;
        }
        else if (string[index] == delimiter)
        {
            in_word = false;
        }
    }
    return (count);
}

static void free_strings(char **strings, ft_size_t filled)
{
    for (ft_size_t index = 0; index < filled; ++index)
    {
        if (strings[index])
            cma_free(strings[index]);
    }
    cma_free(strings);
}

static char *duplicate_range(const char *source, ft_size_t start, ft_size_t length)
{
    char *result = static_cast<char *>(cma_malloc(length + 1));
    if (result == ft_nullptr)
        return (ft_nullptr);
    for (ft_size_t index = 0; index < length; ++index)
        result[index] = source[start + index];
    result[length] = '\0';
    return (result);
}

char **adv_split(char const *string, char delimiter)
{
    if (string == ft_nullptr)
        return (ft_nullptr);
    ft_size_t word_count = count_words(string, delimiter);
    char **result = static_cast<char **>(cma_malloc((word_count + 1) * sizeof(char *)));
    if (result == ft_nullptr)
        return (ft_nullptr);
    ft_size_t current_word = 0;
    ft_size_t index = 0;
    while (string[index] != '\0')
    {
        if (string[index] == delimiter)
        {
            ++index;
            continue;
        }
        ft_size_t start = index;
        while (string[index] != '\0' && string[index] != delimiter)
            ++index;
        ft_size_t length = index - start;
        if (length == 0)
            continue;
        char *word = duplicate_range(string, start, length);
        if (word == ft_nullptr)
        {
            free_strings(result, current_word);
            return (ft_nullptr);
        }
        result[current_word++] = word;
    }
    result[current_word] = ft_nullptr;
    return (result);
}
