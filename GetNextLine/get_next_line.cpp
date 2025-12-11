#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/unordered_map.hpp"
#include "../Errno/errno.hpp"
#include "get_next_line.hpp"
#include "gnl_stream.hpp"
#include <new>
#include <unistd.h>

static bool map_has_new_error(ft_unordered_map<int, char*> &map, int previous_error, int *current_error)
{
    int updated_error;

    updated_error = map.get_error();
    if (current_error)
        *current_error = updated_error;
    if (updated_error != FT_ERR_SUCCESSS)
    {
        if (previous_error == FT_ERR_SUCCESSS)
            return (true);
        if (updated_error != previous_error)
            return (true);
    }
    return (false);
}

static ft_unordered_map<int, char*> g_gnl_leftovers;
static ft_unordered_map<int, gnl_stream*> g_gnl_streams;

static bool stream_map_has_new_error(ft_unordered_map<int, gnl_stream*> &map,
    int previous_error, int *current_error)
{
    int updated_error;

    updated_error = map.get_error();
    if (current_error)
        *current_error = updated_error;
    if (updated_error != FT_ERR_SUCCESSS)
    {
        if (previous_error == FT_ERR_SUCCESSS)
            return (true);
        if (updated_error != previous_error)
            return (true);
    }
    return (false);
}

static gnl_stream *gnl_acquire_stream(int fd, int *stream_error)
{
    int map_error_before;
    int map_error_after;
    ft_unordered_map<int, gnl_stream*>::iterator stream_it = g_gnl_streams.end();
    gnl_stream *existing_stream;
    void *memory;
    gnl_stream *new_stream;
    int init_error;

    if (stream_error)
        *stream_error = FT_ERR_SUCCESSS;
    if (fd < 0)
    {
        if (stream_error)
            *stream_error = FT_ERR_INVALID_ARGUMENT;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    map_error_before = g_gnl_streams.get_error();
    stream_it = g_gnl_streams.find(fd);
    if (stream_map_has_new_error(g_gnl_streams, map_error_before, &map_error_after))
    {
        if (stream_error)
            *stream_error = map_error_after;
        ft_errno = map_error_after;
        return (ft_nullptr);
    }
    if (stream_it != g_gnl_streams.end())
        return (stream_it->second);
    memory = cma_malloc(sizeof(gnl_stream));
    if (!memory)
    {
        if (stream_error)
            *stream_error = FT_ERR_NO_MEMORY;
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    new_stream = new(memory) gnl_stream();
    init_error = new_stream->init_from_fd(fd);
    if (init_error != FT_ERR_SUCCESSS)
    {
        new_stream->~gnl_stream();
        cma_free(memory);
        if (stream_error)
            *stream_error = init_error;
        ft_errno = init_error;
        return (ft_nullptr);
    }
    map_error_before = g_gnl_streams.get_error();
    g_gnl_streams.insert(fd, new_stream);
    if (stream_map_has_new_error(g_gnl_streams, map_error_before, &map_error_after))
    {
        new_stream->reset();
        new_stream->~gnl_stream();
        cma_free(memory);
        if (stream_error)
            *stream_error = map_error_after;
        ft_errno = map_error_after;
        return (ft_nullptr);
    }
    existing_stream = new_stream;
    if (stream_error)
        *stream_error = FT_ERR_SUCCESSS;
    ft_errno = FT_ERR_SUCCESSS;
    return (existing_stream);
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
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    return (new_string);
}

static void    *(*g_gnl_leftover_alloc_hook)(ft_size_t size) = cma_malloc;

void    gnl_set_leftover_alloc_hook(void *(*hook)(ft_size_t size))
{
    if (hook)
        g_gnl_leftover_alloc_hook = hook;
    else
        g_gnl_leftover_alloc_hook = cma_malloc;
    return ;
}

void    gnl_reset_leftover_alloc_hook(void)
{
    g_gnl_leftover_alloc_hook = cma_malloc;
    return ;
}

void    gnl_reset_all_streams(void)
{
    int previous_error;

    previous_error = ft_errno;
    if (g_gnl_leftovers.has_valid_storage())
    {
        ft_unordered_map<int, char*>::iterator map_iterator = g_gnl_leftovers.begin();

        while (map_iterator != g_gnl_leftovers.end())
        {
            char    *buffer;

            buffer = map_iterator->second;
            if (buffer)
                cma_free(buffer);
            ++map_iterator;
        }
        g_gnl_leftovers.clear();
    }
    if (g_gnl_streams.has_valid_storage())
    {
        ft_unordered_map<int, gnl_stream*>::iterator stream_iterator = g_gnl_streams.begin();

        while (stream_iterator != g_gnl_streams.end())
        {
            gnl_stream *stream_pointer;

            stream_pointer = stream_iterator->second;
            if (stream_pointer)
            {
                stream_pointer->reset();
                stream_pointer->~gnl_stream();
                cma_free(stream_pointer);
            }
            ++stream_iterator;
        }
        g_gnl_streams.clear();
    }
    ft_errno = previous_error;
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

static char* leftovers(char* readed_string, bool *buffer_was_freed, int *leftover_error)
{
    int read_index = 0;
    int write_index = 0;
    char* string;

    if (buffer_was_freed)
        *buffer_was_freed = false;
    if (leftover_error)
        *leftover_error = FT_ERR_SUCCESSS;
    while (readed_string[read_index] && readed_string[read_index] != '\n')
        read_index++;
    if (!readed_string[read_index])
    {
        cma_free(readed_string);
        if (buffer_was_freed)
            *buffer_was_freed = true;
        return (ft_nullptr);
    }
    string = static_cast<char*>(g_gnl_leftover_alloc_hook(ft_strlen(readed_string)
            - read_index + 1));
    if (!string)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        if (leftover_error)
            *leftover_error = FT_ERR_NO_MEMORY;
        if (readed_string)
        {
            ft_size_t combined_length;

            combined_length = ft_strlen(readed_string);
            if (combined_length > 0)
                ft_bzero(readed_string, combined_length);
        }
        cma_free(readed_string);
        if (buffer_was_freed)
            *buffer_was_freed = true;
        return (ft_nullptr);
    }
    read_index++;
    while (readed_string[read_index])
        string[write_index++] = readed_string[read_index++];
    string[write_index] = '\0';
    cma_free(readed_string);
    if (buffer_was_freed)
        *buffer_was_freed = true;
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

static char* read_stream(gnl_stream *stream, char* readed_string, std::size_t buffer_size)
{
    char* buffer;
    ssize_t readed_bytes;
    bool has_read_bytes;

    if (!stream)
    {
        if (readed_string)
            cma_free(readed_string);
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
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
        readed_bytes = stream->read(buffer, buffer_size);
        if (readed_bytes < 0)
        {
            int stream_error_code;

            stream_error_code = stream->get_error();
            if (stream_error_code == FT_ERR_SUCCESSS)
                stream_error_code = FT_ERR_IO;
            cma_free(buffer);
            if (readed_string)
            {
                cma_free(readed_string);
            }
            ft_errno = stream_error_code;
            return (ft_nullptr);
        }
        buffer[readed_bytes] = '\0';
        if (readed_bytes > 0)
        {
            char* previous_string;

            has_read_bytes = true;
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
    }
    cma_free(buffer);
    if (!has_read_bytes && !readed_string)
        ft_errno = FT_ERR_END_OF_FILE;
    return (readed_string);
}

int gnl_clear_stream(int fd)
{
    int entry_errno;
    int map_error_before;
    int map_error_after;
    ft_unordered_map<int, char*>::iterator map_it = g_gnl_leftovers.end();
    char *leftover;
    ft_unordered_map<int, gnl_stream*>::iterator stream_it = g_gnl_streams.end();
    gnl_stream *stream_pointer;

    entry_errno = ft_errno;
    map_error_before = g_gnl_leftovers.get_error();
    map_it = g_gnl_leftovers.find(fd);
    if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
    {
        ft_errno = map_error_after;
        return (map_error_after);
    }
    if (map_it == g_gnl_leftovers.end())
    {
        ft_errno = entry_errno;
        return (FT_ERR_SUCCESSS);
    }
    leftover = map_it->second;
    map_error_before = g_gnl_leftovers.get_error();
    g_gnl_leftovers.erase(fd);
    if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
    {
        if (leftover)
            cma_free(leftover);
        ft_errno = map_error_after;
        return (map_error_after);
    }
    if (leftover)
        cma_free(leftover);
    map_error_before = g_gnl_streams.get_error();
    stream_it = g_gnl_streams.find(fd);
    if (stream_map_has_new_error(g_gnl_streams, map_error_before, &map_error_after))
    {
        ft_errno = map_error_after;
        return (map_error_after);
    }
    if (stream_it != g_gnl_streams.end())
    {
        stream_pointer = stream_it->second;
        map_error_before = g_gnl_streams.get_error();
        g_gnl_streams.erase(fd);
        if (stream_map_has_new_error(g_gnl_streams, map_error_before, &map_error_after))
        {
            ft_errno = map_error_after;
            return (map_error_after);
        }
        if (stream_pointer)
        {
            stream_pointer->reset();
            stream_pointer->~gnl_stream();
            cma_free(stream_pointer);
        }
    }
    ft_errno = entry_errno;
    return (FT_ERR_SUCCESSS);
}

char    *get_next_line(int fd, std::size_t buffer_size)
{
    char                                   *line;
    char                                   *combined_buffer;
    char                                   *leftover_string;
    bool                                    combined_buffer_was_freed;
    int                                     map_error_before;
    int                                     map_error_after;
    int                                     leftovers_error;
    gnl_stream                              *stream;
    int                                     stream_error;

    line = ft_nullptr;
    combined_buffer = ft_nullptr;
    leftover_string = ft_nullptr;
    if (buffer_size == 0 || fd < 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    stream = gnl_acquire_stream(fd, &stream_error);
    if (!stream)
    {
        if (stream_error != FT_ERR_SUCCESSS)
            ft_errno = stream_error;
        return (ft_nullptr);
    }
    map_error_before = g_gnl_leftovers.get_error();
    ft_unordered_map<int, char*>::iterator map_it = g_gnl_leftovers.find(fd);
    if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
    {
        ft_errno = map_error_after;
        return (ft_nullptr);
    }
    if (map_it != g_gnl_leftovers.end())
    {
        combined_buffer = map_it->second;
        map_error_before = g_gnl_leftovers.get_error();
        g_gnl_leftovers.erase(fd);
        if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
        {
            if (combined_buffer)
                cma_free(combined_buffer);
            ft_errno = map_error_after;
            return (ft_nullptr);
        }
    }
    combined_buffer = read_stream(stream, combined_buffer, buffer_size);
    if (!combined_buffer)
        return (ft_nullptr);
    line = fetch_line(combined_buffer);
    int line_error = ft_errno;
    combined_buffer_was_freed = false;
    leftover_string = leftovers(combined_buffer, &combined_buffer_was_freed, &leftovers_error);
    if (leftovers_error != FT_ERR_SUCCESSS)
    {
        if (line)
            cma_free(line);
        if (combined_buffer && !combined_buffer_was_freed)
        {
            ft_size_t combined_length;

            combined_length = ft_strlen(combined_buffer);
            if (combined_length > 0)
                ft_bzero(combined_buffer, combined_length);
            cma_free(combined_buffer);
        }
        ft_errno = leftovers_error;
        return (ft_nullptr);
    }
    if (!line)
    {
        if (leftover_string)
        {
            map_error_before = g_gnl_leftovers.get_error();
            g_gnl_leftovers.insert(fd, leftover_string);
            if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
            {
                cma_free(leftover_string);
                ft_errno = map_error_after;
                return (ft_nullptr);
            }
        }
        if (line_error != FT_ERR_SUCCESSS)
            ft_errno = line_error;
        return (ft_nullptr);
    }
    if (leftover_string)
    {
        map_error_before = g_gnl_leftovers.get_error();
        g_gnl_leftovers.insert(fd, leftover_string);
        if (map_has_new_error(g_gnl_leftovers, map_error_before, &map_error_after))
        {
            cma_free(leftover_string);
            cma_free(line);
            ft_errno = map_error_after;
            return (ft_nullptr);
        }
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (line);
}
