#include "demo_game.hpp"

#include "../Modules/CMA/CMA.hpp"
#include "../Modules/Basic/class_nullptr.hpp"
#include "../Modules/DUMB/dumb_io.hpp"
#include <cstdio>
#include <cstring>

void demo_leaderboard_reset(demo_leaderboard *leaderboard)
{
    uint32_t index_entry;

    leaderboard->entry_count = 0U;
    index_entry = 0U;
    while (index_entry < 5U)
    {
        leaderboard->entries[index_entry].name[0] = 'A';
        leaderboard->entries[index_entry].name[1] = 'A';
        leaderboard->entries[index_entry].name[2] = 'A';
        leaderboard->entries[index_entry].name[3] = '\0';
        leaderboard->entries[index_entry].elapsed_milliseconds = 0U;
        index_entry = index_entry + 1U;
    }
    return ;
}

void demo_leaderboard_insert(demo_leaderboard *leaderboard, const char *name,
    uint32_t elapsed_milliseconds)
{
    demo_score_entry new_entry;
    uint32_t index_entry;
    uint32_t insert_index;
    uint32_t move_index;

    new_entry.name[0] = name[0];
    new_entry.name[1] = name[1];
    new_entry.name[2] = name[2];
    new_entry.name[3] = '\0';
    new_entry.elapsed_milliseconds = elapsed_milliseconds;
    insert_index = leaderboard->entry_count;
    index_entry = 0U;
    while (index_entry < leaderboard->entry_count)
    {
        if (elapsed_milliseconds < leaderboard->entries[index_entry].elapsed_milliseconds)
        {
            insert_index = index_entry;
            break ;
        }
        index_entry = index_entry + 1U;
    }
    if (leaderboard->entry_count < 5U)
    {
        leaderboard->entry_count = leaderboard->entry_count + 1U;
    }
    move_index = leaderboard->entry_count;
    while (move_index > insert_index + 1U)
    {
        leaderboard->entries[move_index - 1U] = leaderboard->entries[move_index - 2U];
        move_index = move_index - 1U;
    }
    if (insert_index < 5U)
    {
        leaderboard->entries[insert_index] = new_entry;
    }
    return ;
}

void demo_leaderboard_save(const demo_leaderboard *leaderboard)
{
    char        buffer[256];
    uint32_t    index_entry;
    int32_t     write_index;
    int32_t     line_length;

    std::memset(buffer, 0, sizeof(buffer));
    write_index = 0;
    index_entry = 0U;
    while (index_entry < leaderboard->entry_count)
    {
        line_length = std::snprintf(buffer + write_index,
            sizeof(buffer) - static_cast<ft_size_t>(write_index),
            "%c%c%c %u\n",
            leaderboard->entries[index_entry].name[0],
            leaderboard->entries[index_entry].name[1],
            leaderboard->entries[index_entry].name[2],
            leaderboard->entries[index_entry].elapsed_milliseconds);
        if (line_length <= 0)
        {
            return ;
        }
        write_index = write_index + line_length;
        if (write_index >= static_cast<int32_t>(sizeof(buffer)))
        {
            break ;
        }
        index_entry = index_entry + 1U;
    }
    (void)ft_write_file("Demo/demo_leaderboard.txt", buffer,
        static_cast<ft_size_t>(write_index));
    return ;
}

static uint32_t demo_parse_u32(const char *buffer, ft_size_t *index_pointer,
    ft_size_t size)
{
    uint32_t value;

    value = 0U;
    while (*index_pointer < size && buffer[*index_pointer] >= '0'
        && buffer[*index_pointer] <= '9')
    {
        value = (value * 10U)
            + static_cast<uint32_t>(buffer[*index_pointer] - '0');
        *index_pointer = *index_pointer + 1U;
    }
    return (value);
}

void demo_leaderboard_load(demo_leaderboard *leaderboard)
{
    char        *raw_buffer;
    ft_size_t   buffer_size;
    ft_size_t   index_buffer;
    char        name_buffer[4];
    uint32_t    elapsed_milliseconds;
    int32_t     read_error;

    demo_leaderboard_reset(leaderboard);
    raw_buffer = ft_nullptr;
    buffer_size = 0U;
    read_error = ft_read_file("Demo/demo_leaderboard.txt", &raw_buffer, &buffer_size);
    if (read_error != FT_ERR_SUCCESS || raw_buffer == ft_nullptr)
    {
        return ;
    }
    index_buffer = 0U;
    while (index_buffer + 4U < buffer_size && leaderboard->entry_count < 5U)
    {
        if ((raw_buffer[index_buffer] < 'A' || raw_buffer[index_buffer] > 'Z')
            || (raw_buffer[index_buffer + 1U] < 'A' || raw_buffer[index_buffer + 1U] > 'Z')
            || (raw_buffer[index_buffer + 2U] < 'A' || raw_buffer[index_buffer + 2U] > 'Z'))
        {
            while (index_buffer < buffer_size && raw_buffer[index_buffer] != '\n')
            {
                index_buffer = index_buffer + 1U;
            }
            if (index_buffer < buffer_size)
            {
                index_buffer = index_buffer + 1U;
            }
            continue ;
        }
        name_buffer[0] = raw_buffer[index_buffer];
        name_buffer[1] = raw_buffer[index_buffer + 1U];
        name_buffer[2] = raw_buffer[index_buffer + 2U];
        name_buffer[3] = '\0';
        index_buffer = index_buffer + 3U;
        if (index_buffer >= buffer_size || raw_buffer[index_buffer] != ' ')
        {
            break ;
        }
        index_buffer = index_buffer + 1U;
        elapsed_milliseconds = demo_parse_u32(raw_buffer, &index_buffer, buffer_size);
        demo_leaderboard_insert(leaderboard, name_buffer, elapsed_milliseconds);
        while (index_buffer < buffer_size && raw_buffer[index_buffer] != '\n')
        {
            index_buffer = index_buffer + 1U;
        }
        if (index_buffer < buffer_size)
        {
            index_buffer = index_buffer + 1U;
        }
    }
    cma_free(raw_buffer);
    return ;
}
