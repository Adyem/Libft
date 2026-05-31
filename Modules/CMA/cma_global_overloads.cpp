#include <cstddef>
#include <new>
#include "../CMA/CMA.hpp"
#include "../Basic/class_nullptr.hpp"

void* operator new(std::size_t size)
{
    void *pointer;

    pointer = cma_malloc(size);
    if (pointer == ft_nullptr)
        throw std::bad_alloc();
    return (pointer);
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept
{
    void *pointer;

    pointer = cma_malloc(size);
    return (pointer);
}

void operator delete(void* memory_pointer) noexcept
{
    cma_free(memory_pointer);
    return ;
}

void operator delete(void* memory_pointer, std::size_t) noexcept
{
    cma_free(memory_pointer);
    return ;
}

void operator delete(void* memory_pointer, const std::nothrow_t&) noexcept
{
    cma_free(memory_pointer);
    return ;
}

void* operator new[](std::size_t size)
{
    void *pointer;

    pointer = cma_malloc(size);
    if (pointer == ft_nullptr)
        throw std::bad_alloc();
    return (pointer);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
    void *pointer;

    pointer = cma_malloc(size);
    return (pointer);
}

void operator delete[](void* memory_pointer) noexcept
{
    cma_free(memory_pointer);
    return ;
}

void operator delete[](void* memory_pointer, std::size_t) noexcept
{
    cma_free(memory_pointer);
    return ;
}

void operator delete[](void* memory_pointer, const std::nothrow_t&) noexcept
{
    cma_free(memory_pointer);
    return ;
}

void* operator new(std::size_t size, std::align_val_t alignment)
{
    void *pointer;

    pointer = cma_aligned_alloc(static_cast<ft_size_t>(alignment),
            size);
    if (pointer == ft_nullptr)
        throw std::bad_alloc();
    return (pointer);
}

void* operator new(std::size_t size, std::align_val_t alignment,
    const std::nothrow_t&) noexcept
{
    return (cma_aligned_alloc(static_cast<ft_size_t>(alignment),
            size));
}

void operator delete(void* memory_pointer, std::align_val_t) noexcept
{
    cma_free(memory_pointer);
    return ;
}

void operator delete(void* memory_pointer, std::size_t, std::align_val_t) noexcept
{
    cma_free(memory_pointer);
    return ;
}

void operator delete(void* memory_pointer, std::align_val_t, const std::nothrow_t&) noexcept
{
    cma_free(memory_pointer);
    return ;
}

void* operator new[](std::size_t size, std::align_val_t alignment)
{
   void *pointer;

    pointer = cma_aligned_alloc(static_cast<ft_size_t>(alignment),
            size);
    if (pointer == ft_nullptr)
        throw std::bad_alloc();
    return (pointer);
}

void* operator new[](std::size_t size, std::align_val_t alignment,
    const std::nothrow_t&) noexcept
{
    return (cma_aligned_alloc(static_cast<ft_size_t>(alignment),
            size));
}

void operator delete[](void* memory_pointer, std::align_val_t) noexcept
{
    cma_free(memory_pointer);
    return ;
}

void operator delete[](void* memory_pointer, std::size_t, std::align_val_t) noexcept
{
    cma_free(memory_pointer);
    return ;
}

void operator delete[](void* memory_pointer, std::align_val_t,
    const std::nothrow_t&) noexcept
{
    cma_free(memory_pointer);
    return ;
}
