#ifndef PTHREAD_BUFFER_HPP
# define PTHREAD_BUFFER_HPP

# include <cstdlib>
# include "../CPP_class/class_nullptr.hpp"

template <typename t_type>
struct pt_buffer
{
    t_type    *data;
    ft_size_t  size;
    ft_size_t  capacity;
};

template <typename t_type>
void pt_buffer_init(pt_buffer<t_type> &buffer)
{
    buffer.data = ft_nullptr;
    buffer.size = 0;
    buffer.capacity = 0;
}

template <typename t_type>
void pt_buffer_destroy(pt_buffer<t_type> &buffer)
{
    if (buffer.data != ft_nullptr)
    {
        std::free(buffer.data);
        buffer.data = ft_nullptr;
    }
    buffer.size = 0;
    buffer.capacity = 0;
}

template <typename t_type>
int pt_buffer_reserve(pt_buffer<t_type> &buffer, ft_size_t required)
{
    ft_size_t new_capacity;
    void *new_data;
    ft_size_t copy_index;

    if (required <= buffer.capacity)
        return (FT_ERR_SUCCESS);
    if (buffer.capacity != 0)
        new_capacity = buffer.capacity * 2;
    else
        new_capacity = 4;
    if (new_capacity < required)
        new_capacity = required;
    new_data = std::malloc(new_capacity * sizeof(t_type));
    if (new_data == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    copy_index = 0;
    while (copy_index < buffer.size)
    {
        static_cast<t_type *>(new_data)[copy_index] = buffer.data[copy_index];
        copy_index += 1;
    }
    if (buffer.data != ft_nullptr)
        std::free(buffer.data);
    buffer.data = static_cast<t_type *>(new_data);
    buffer.capacity = new_capacity;
    return (FT_ERR_SUCCESS);
}

template <typename t_type>
int pt_buffer_push(pt_buffer<t_type> &buffer, const t_type &value)
{
    int reserve_error = pt_buffer_reserve(buffer, buffer.size + 1);
    if (reserve_error != FT_ERR_SUCCESS)
        return (reserve_error);
    buffer.data[buffer.size] = value;
    buffer.size += 1;
    return (FT_ERR_SUCCESS);
}

template <typename t_type>
void pt_buffer_clear(pt_buffer<t_type> &buffer)
{
    buffer.size = 0;
}

template <typename t_type>
int pt_buffer_copy(pt_buffer<t_type> &dest,
        const pt_buffer<t_type> &source)
{
    int reserve_error = pt_buffer_reserve(dest, source.size);
    if (reserve_error != FT_ERR_SUCCESS)
        return (reserve_error);
    ft_size_t copy_index = 0;
    while (copy_index < source.size)
    {
        dest.data[copy_index] = source.data[copy_index];
        copy_index += 1;
    }
    dest.size = source.size;
    return (FT_ERR_SUCCESS);
}

template <typename t_type>
void pt_buffer_erase(pt_buffer<t_type> &buffer, ft_size_t index)
{
    if (index >= buffer.size)
        return ;
    ft_size_t move_index = index;
    while (move_index + 1 < buffer.size)
    {
        buffer.data[move_index] = buffer.data[move_index + 1];
        move_index += 1;
    }
    buffer.size -= 1;
}

#endif
