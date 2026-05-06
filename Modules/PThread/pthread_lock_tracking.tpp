#ifndef PTHREAD_LOCK_TRACKING_TPP
# define PTHREAD_LOCK_TRACKING_TPP

template <typename t_type>
pt_system_allocator<t_type>::pt_system_allocator()
{
    return ;
}

template <typename t_type>
template <typename t_other>
pt_system_allocator<t_type>::pt_system_allocator(const pt_system_allocator<t_other> &other)
{
    (void)other;
    return ;
}

template <typename t_type>
pt_system_allocator<t_type>::~pt_system_allocator()
{
    return ;
}

template <typename t_type>
t_type *pt_system_allocator<t_type>::allocate(std::size_t count)
{
    void *memory_pointer;

    memory_pointer = std::malloc(count * sizeof(t_type));
    if (memory_pointer == ft_nullptr)
        throw std::bad_alloc();
    return (static_cast<t_type *>(memory_pointer));
}

template <typename t_type>
void pt_system_allocator<t_type>::deallocate(t_type *pointer, std::size_t count)
{
    (void)count;
    if (pointer == ft_nullptr)
        return ;
    std::free(pointer);
    return ;
}

template <typename t_type, typename t_other>
bool operator==(const pt_system_allocator<t_type> &left, const pt_system_allocator<t_other> &right)
{
    (void)left;
    (void)right;
    return (true);
}

template <typename t_type, typename t_other>
bool operator!=(const pt_system_allocator<t_type> &left, const pt_system_allocator<t_other> &right)
{
    (void)left;
    (void)right;
    return (false);
}

#endif
