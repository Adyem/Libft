#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdio>
#include "../CPP_class/class_istream.hpp"
#include "../Template/unordened_map.hpp"
#include "../Errno/errno.hpp"
#include "get_next_line.hpp"

static bool map_has_new_error(ft_unord_map<ft_istream*, char*> &map, int previous_error, int *current_error)
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
        ft_errno = FT_EALLOC;
        return (ft_nullptr);
    }
    return (new_string);
}

char* ft_strjoin_gnl(char* string_one, char* string_two)
{
    char* result;
    char* original_string = string_one;
    int index;

    if (!string_one && !string_two)
    {
        ft_errno = FT_EINVAL;
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

    while (readed_string[read_index] && readed_string[read_index] != '\n')
        read_index++;
    if (!readed_string[read_index])
    {
        cma_free(readed_string);
        return (ft_nullptr);
    }
    string = static_cast<char*>(cma_malloc(ft_strlen(readed_string) - read_index + 1));
    if (!string)
    {
        ft_errno = FT_EALLOC;
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
        ft_errno = FT_EALLOC;
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
        ft_errno = FILE_END_OF_FILE;
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

    buffer = static_cast<char*>(cma_malloc(buffer_size + 1));
    if (!buffer)
    {
        ft_errno = FT_EALLOC;
        return (ft_nullptr);
    }
    readed_bytes = 1;
    has_read_bytes = false;
    while (!ft_strchr(readed_string, '\n') && readed_bytes != 0)
    {
        input.read(buffer, buffer_size);
        readed_bytes = input.gcount();
        if (readed_bytes > 0)
            has_read_bytes = true;
        if (input.bad())
        {
            cma_free(buffer);
            cma_free(readed_string);
            int stream_error = input.get_error();
            if (stream_error == ER_SUCCESS)
                ft_errno = FT_EIO;
            else
                ft_errno = stream_error;
            return (ft_nullptr);
        }
        buffer[readed_bytes] = '\0';
        readed_string = ft_strjoin_gnl(readed_string, buffer);
        if (!readed_string)
        {
            cma_free(buffer);
            return (ft_nullptr);
        }
    }
    cma_free(buffer);
    if (!has_read_bytes && !readed_string)
        ft_errno = FILE_END_OF_FILE;
    return (readed_string);
}

char    *get_next_line(ft_istream &input, std::size_t buffer_size)
{
    static ft_unord_map<ft_istream*, char*> readed_map;
    char                                   *string = ft_nullptr;
    char                                   *stored_string = ft_nullptr;
    int                                     map_error_before;
    int                                     map_error_after;

    if (buffer_size == 0)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    map_error_before = readed_map.get_error();
    ft_unord_map<ft_istream*, char*>::iterator map_it = readed_map.find(&input);
    if (map_has_new_error(readed_map, map_error_before, &map_error_after))
    {
        ft_errno = map_error_after;
        return (ft_nullptr);
    }
    if (map_it != readed_map.end())
        stored_string = map_it->second;
    stored_string = read_stream(input, stored_string, buffer_size);
    if (!stored_string)
    {
        int failure_errno;

        failure_errno = ft_errno;
        map_error_before = readed_map.get_error();
        readed_map.remove(&input);
        if (map_has_new_error(readed_map, map_error_before, &map_error_after))
        {
            ft_errno = map_error_after;
            return (ft_nullptr);
        }
        if (failure_errno != ER_SUCCESS)
            ft_errno = failure_errno;
        return (ft_nullptr);
    }
    string = fetch_line(stored_string);
    int line_error = ft_errno;
    stored_string = leftovers(stored_string);
    int leftovers_error = ft_errno;
    if (!string)
    {
        if (stored_string)
        {
            map_error_before = readed_map.get_error();
            readed_map.insert(&input, stored_string);
            if (map_has_new_error(readed_map, map_error_before, &map_error_after))
            {
                cma_free(stored_string);
                ft_errno = map_error_after;
                return (ft_nullptr);
            }
        }
        else
        {
            map_error_before = readed_map.get_error();
            readed_map.remove(&input);
            if (map_has_new_error(readed_map, map_error_before, &map_error_after))
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
        map_error_before = readed_map.get_error();
        readed_map.remove(&input);
        if (map_has_new_error(readed_map, map_error_before, &map_error_after))
        {
            ft_errno = map_error_after;
            cma_free(string);
            return (ft_nullptr);
        }
        if (leftovers_error != ER_SUCCESS)
        {
            cma_free(string);
            ft_errno = leftovers_error;
            return (ft_nullptr);
        }
    }
    else
    {
        map_error_before = readed_map.get_error();
        readed_map.insert(&input, stored_string);
        if (map_has_new_error(readed_map, map_error_before, &map_error_after))
        {
            cma_free(stored_string);
            cma_free(string);
            ft_errno = map_error_after;
            return (ft_nullptr);
        }
    }
    ft_errno = ER_SUCCESS;
    return (string);
}
