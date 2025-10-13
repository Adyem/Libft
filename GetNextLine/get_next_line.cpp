#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdio>
#include "../CPP_class/class_istream.hpp"
#include "../Template/unordered_map.hpp"
#include "../Errno/errno.hpp"
#include "get_next_line.hpp"

struct gnl_leftover_entry
{
    char    *buffer;
};

static bool map_has_new_error(ft_unordered_map<ft_istream*, gnl_leftover_entry*> &map, int previous_error, int *current_error)
{
    int updated_error;

    updated_error = map.get_error();
    if (current_error)
        *current_error = updated_error;
    if (updated_error != ER_SUCCESS)
    {
        if (previous_error == ER_SUCCESS)
            return (true);
        if (updated_error != previous_error)
            return (true);
    }
    return (false);
}

static ft_unordered_map<ft_istream*, gnl_leftover_entry*> g_gnl_leftovers;

static char* allocate_new_string(char* string_one, char* string_two)
{
    int total_length = 0;
    char* new_string;

    if (string_one)
        total_length += ft_strlen(string_one);
    if (string_two)
        total_length += ft_strlen(string_two);
    new_string = static_cast<char*>(cma_malloc(total_length + 1));
    if (!new_string)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    return (new_string);
}

static void    *(*g_gnl_leftover_alloc_hook)(ft_size_t size) = cma_malloc;
static int      g_gnl_last_leftover_error = ER_SUCCESS;

void    gnl_set_leftover_alloc_hook(void *(*hook)(ft_size_t size))
{
    if (hook)
        g_gnl_leftover_alloc_hook = hook;
    else
        g_gnl_leftover_alloc_hook = cma_malloc;
    g_gnl_last_leftover_error = ER_SUCCESS;
    return ;
}

void    gnl_reset_leftover_alloc_hook(void)
{
    g_gnl_leftover_alloc_hook = cma_malloc;
    g_gnl_last_leftover_error = ER_SUCCESS;
    return ;
}

char* ft_strjoin_gnl(char* string_one, char* string_two)
{
    char* result;
    char* original_string = string_one;
    int index;

    if (!string_one && !string_two)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    result = allocate_new_string(string_one, string_two);
    if (!result)
        return (ft_nullptr);
    index = 0;
    if (string_one)
        while (*string_one)
            result[index++] = *string_one++;
    if (string_two)
        while (*string_two)
            result[index++] = *string_two++;
    result[index] = '\0';
    cma_free(original_string);
    return (result);
}

static char* leftovers(char* readed_string)
{
    int read_index = 0;
    int write_index = 0;
    char* string;

    g_gnl_last_leftover_error = ER_SUCCESS;
    while (readed_string[read_index] && readed_string[read_index] != '\n')
        read_index++;
    if (!readed_string[read_index])
    {
        cma_free(readed_string);
        return (ft_nullptr);
    }
    string = static_cast<char*>(g_gnl_leftover_alloc_hook(ft_strlen(readed_string)
            - read_index + 1));
    if (!string)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        g_gnl_last_leftover_error = FT_ERR_NO_MEMORY;
        cma_free(readed_string);
        return (ft_nullptr);
    }
    read_index++;
    while (readed_string[read_index])
        string[write_index++] = readed_string[read_index++];
    string[write_index] = '\0';
    cma_free(readed_string);
    return (string);
}

static char* malloc_gnl(char* readed_string, size_t length)
{
    char* string;

    if (readed_string && readed_string[length] == '\n')
        string = static_cast<char*>(cma_malloc(length + 2));
    else
        string = static_cast<char*>(cma_malloc(length + 1));
    if (!string)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    return (string);
}

static char* fetch_line(char* readed_string)
{
    size_t index = 0;
    char* string;

    if (!readed_string[index])
    {
        ft_errno = FT_ERR_END_OF_FILE;
        return (ft_nullptr);
    }
    while (readed_string[index] && readed_string[index] != '\n')
        index++;
    string = malloc_gnl(readed_string, index);
    if (!string)
        return (ft_nullptr);
    index = 0;
    while (readed_string[index] && readed_string[index] != '\n')
    {
        string[index] = readed_string[index];
        index++;
    }
    if (readed_string[index] == '\n')
    {
        string[index] = '\n';
        index++;
    }
    string[index] = '\0';
    return (string);
}

static char* read_stream(ft_istream &input, char* readed_string, std::size_t buffer_size)
{
    char* buffer;
    std::size_t readed_bytes;
    bool has_read_bytes;

    if (readed_string && ft_strchr(readed_string, '\n'))
    {
        return (readed_string);
    }
    buffer = static_cast<char*>(cma_malloc(buffer_size + 1));
    if (!buffer)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        if (readed_string)
        {
            cma_free(readed_string);
        }
        return (ft_nullptr);
    }
    readed_bytes = 1;
    has_read_bytes = false;
    while (((readed_string == ft_nullptr) || !ft_strchr(readed_string, '\n'))
        && readed_bytes != 0)
    {
        input.read(buffer, buffer_size);
        readed_bytes = input.gcount();
        if (readed_bytes > 0)
            has_read_bytes = true;
        if (input.bad())
        {
            cma_free(buffer);
            if (readed_string)
            {
                cma_free(readed_string);
            }
            int stream_error = input.get_error();
            if (stream_error == ER_SUCCESS)
                ft_errno = FT_ERR_IO;
            else
                ft_errno = stream_error;
            return (ft_nullptr);
        }
        buffer[readed_bytes] = '\0';
        char* previous_string;

        previous_string = readed_string;
        readed_string = ft_strjoin_gnl(readed_string, buffer);
        if (!readed_string)
        {
            if (previous_string)
            {
                cma_free(previous_string);
            }
            cma_free(buffer);
            return (ft_nullptr);
        }
    }
    cma_free(buffer);
    if (!has_read_bytes && !readed_string)
        ft_errno = FT_ERR_END_OF_FILE;
    return (readed_string);
}

int gnl_clear_stream(ft_istream &input)
{
    int entry_errno;
    int map_error_before;
    int map_error_after;
    ft_unordered_map<ft_istream*, gnl_leftover_entry*>::iterator map_it = g_gnl_leftovers.end();
    gnl_leftover_entry *entry;
    char *leftover;

    entry_errno = ft_errno;
    map_error_before = g_gnl_leftovers.get_error();
    map_it = g_gnl_leftovers.find(&input);
    if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
    {
        ft_errno = map_error_after;
        return (map_error_after);
    }
    if (map_it == g_gnl_leftovers.end())
    {
        ft_errno = entry_errno;
        return (ER_SUCCESS);
    }
    entry = map_it->second;
    leftover = ft_nullptr;
    if (entry)
        leftover = entry->buffer;
    map_error_before = g_gnl_leftovers.get_error();
    g_gnl_leftovers.erase(&input);
    if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
    {
        if (leftover)
            cma_free(leftover);
        if (entry)
            cma_free(entry);
        ft_errno = map_error_after;
        return (map_error_after);
    }
    if (leftover)
        cma_free(leftover);
    if (entry)
        cma_free(entry);
    ft_errno = entry_errno;
    return (ER_SUCCESS);
}

char    *get_next_line(ft_istream &input, std::size_t buffer_size)
{
    char                                   *string = ft_nullptr;
    char                                   *stored_string = ft_nullptr;
    gnl_leftover_entry                     *stored_entry = ft_nullptr;
    int                                     map_error_before;
    int                                     map_error_after;

    if (buffer_size == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    map_error_before = g_gnl_leftovers.get_error();
    ft_unordered_map<ft_istream*, gnl_leftover_entry*>::iterator map_it = g_gnl_leftovers.find(&input);
    if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
    {
        ft_errno = map_error_after;
        return (ft_nullptr);
    }
    if (map_it != g_gnl_leftovers.end())
    {
        stored_entry = map_it->second;
        if (stored_entry)
            stored_string = stored_entry->buffer;
        map_error_before = g_gnl_leftovers.get_error();
        g_gnl_leftovers.erase(&input);
        if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
        {
            ft_errno = map_error_after;
            return (ft_nullptr);
        }
    }
    stored_string = read_stream(input, stored_string, buffer_size);
    if (!stored_string)
    {
        int failure_errno;

        if (stored_entry)
        {
            cma_free(stored_entry);
        }
        failure_errno = ft_errno;
        if (failure_errno != ER_SUCCESS)
            ft_errno = failure_errno;
        return (ft_nullptr);
    }
    string = fetch_line(stored_string);
    int line_error = ft_errno;
    if (stored_entry)
        stored_entry->buffer = ft_nullptr;
    stored_string = leftovers(stored_string);
    int leftovers_error = g_gnl_last_leftover_error;
    if (leftovers_error != ER_SUCCESS)
    {
        if (stored_entry)
        {
            cma_free(stored_entry);
            stored_entry = ft_nullptr;
        }
        map_error_before = g_gnl_leftovers.get_error();
        g_gnl_leftovers.erase(&input);
        if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
        {
            ft_errno = map_error_after;
            cma_free(string);
            return (ft_nullptr);
        }
        cma_free(string);
        ft_errno = leftovers_error;
        return (ft_nullptr);
    }
    if (!string)
    {
        if (stored_string)
        {
            gnl_leftover_entry *entry_to_store;

            entry_to_store = stored_entry;
            if (!entry_to_store)
            {
                entry_to_store = static_cast<gnl_leftover_entry*>(cma_malloc(sizeof(gnl_leftover_entry)));
                if (!entry_to_store)
                {
                    cma_free(stored_string);
                    ft_errno = FT_ERR_NO_MEMORY;
                    return (ft_nullptr);
                }
            }
            entry_to_store->buffer = stored_string;
            map_error_before = g_gnl_leftovers.get_error();
            g_gnl_leftovers.insert(&input, entry_to_store);
            if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
            {
                cma_free(stored_string);
                cma_free(entry_to_store);
                ft_errno = map_error_after;
                return (ft_nullptr);
            }
        }
        else if (stored_entry)
        {
            cma_free(stored_entry);
            stored_entry = ft_nullptr;
            map_error_before = g_gnl_leftovers.get_error();
            g_gnl_leftovers.erase(&input);
            if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
            {
                ft_errno = map_error_after;
                return (ft_nullptr);
            }
        }
        else
        {
            map_error_before = g_gnl_leftovers.get_error();
            g_gnl_leftovers.erase(&input);
            if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
            {
                ft_errno = map_error_after;
                return (ft_nullptr);
            }
        }
        if (line_error != ER_SUCCESS)
            ft_errno = line_error;
        return (ft_nullptr);
    }
    if (!stored_string)
    {
        if (stored_entry)
        {
            cma_free(stored_entry);
            stored_entry = ft_nullptr;
        }
        map_error_before = g_gnl_leftovers.get_error();
        g_gnl_leftovers.erase(&input);
        if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
        {
            ft_errno = map_error_after;
            cma_free(string);
            return (ft_nullptr);
        }
    }
    else
    {
        gnl_leftover_entry *entry_to_store;

        entry_to_store = stored_entry;
        if (!entry_to_store)
        {
            entry_to_store = static_cast<gnl_leftover_entry*>(cma_malloc(sizeof(gnl_leftover_entry)));
            if (!entry_to_store)
            {
                cma_free(stored_string);
                cma_free(string);
                ft_errno = FT_ERR_NO_MEMORY;
                return (ft_nullptr);
            }
        }
        entry_to_store->buffer = stored_string;
        map_error_before = g_gnl_leftovers.get_error();
        g_gnl_leftovers.insert(&input, entry_to_store);
        if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
        {
            cma_free(stored_string);
            cma_free(entry_to_store);
            cma_free(string);
            ft_errno = map_error_after;
            return (ft_nullptr);
        }
    }
    ft_errno = ER_SUCCESS;
    return (string);
}
