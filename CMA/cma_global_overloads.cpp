#include <cstddef>
#include <new>
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

void* operator new(std::size_t size)
{
    int previous_errno;
    void *pointer;

    previous_errno = ft_errno;
    pointer = cma_malloc(size);
    if (pointer == NULL)
        throw std::bad_alloc();
    ft_errno = previous_errno;
    return (pointer);
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept
{
    int previous_errno;
    void *pointer;

    previous_errno = ft_errno;
    pointer = cma_malloc(size);
    if (pointer != NULL)
        ft_errno = previous_errno;
    return (pointer);
}

void operator delete(void* ptr) noexcept
{
    cma_free(ptr);
    return ;
}

void operator delete(void* ptr, std::size_t) noexcept
{
    cma_free(ptr);
    return ;
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept
{
    cma_free(ptr);
    return ;
}

void* operator new[](std::size_t size)
{
    int previous_errno;
    void *pointer;

    previous_errno = ft_errno;
    pointer = cma_malloc(size);
    if (pointer == NULL)
        throw std::bad_alloc();
    ft_errno = previous_errno;
    return (pointer);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
    int previous_errno;
    void *pointer;

    previous_errno = ft_errno;
    pointer = cma_malloc(size);
    if (pointer != NULL)
        ft_errno = previous_errno;
    return (pointer);
}

void operator delete[](void* ptr) noexcept
{
    cma_free(ptr);
    return ;
}

void operator delete[](void* ptr, std::size_t) noexcept
{
    cma_free(ptr);
    return ;
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept
{
    cma_free(ptr);
    return ;
}
