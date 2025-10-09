#include <cstddef>
#include <new>
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

static void *allocate_aligned_memory(std::size_t size, std::align_val_t alignment)
{
    int previous_errno;
    void *pointer;

    previous_errno = ft_errno;
    pointer = cma_aligned_alloc(static_cast<ft_size_t>(alignment),
        static_cast<ft_size_t>(size));
    if (pointer != NULL)
        ft_errno = previous_errno;
    return (pointer);
}

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

void* operator new(std::size_t size, std::align_val_t alignment)
{
    void *pointer;

    pointer = allocate_aligned_memory(size, alignment);
    if (pointer == NULL)
        throw std::bad_alloc();
    return (pointer);
}

void* operator new(std::size_t size, std::align_val_t alignment,
    const std::nothrow_t&) noexcept
{
    return (allocate_aligned_memory(size, alignment));
}

void operator delete(void* ptr, std::align_val_t) noexcept
{
    cma_free(ptr);
    return ;
}

void operator delete(void* ptr, std::size_t, std::align_val_t) noexcept
{
    cma_free(ptr);
    return ;
}

void operator delete(void* ptr, std::align_val_t, const std::nothrow_t&) noexcept
{
    cma_free(ptr);
    return ;
}

void* operator new[](std::size_t size, std::align_val_t alignment)
{
    void *pointer;

    pointer = allocate_aligned_memory(size, alignment);
    if (pointer == NULL)
        throw std::bad_alloc();
    return (pointer);
}

void* operator new[](std::size_t size, std::align_val_t alignment,
    const std::nothrow_t&) noexcept
{
    return (allocate_aligned_memory(size, alignment));
}

void operator delete[](void* ptr, std::align_val_t) noexcept
{
    cma_free(ptr);
    return ;
}

void operator delete[](void* ptr, std::size_t, std::align_val_t) noexcept
{
    cma_free(ptr);
    return ;
}

void operator delete[](void* ptr, std::align_val_t,
    const std::nothrow_t&) noexcept
{
    cma_free(ptr);
    return ;
}
