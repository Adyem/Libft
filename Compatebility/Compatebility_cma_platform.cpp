#include "compatebility_cma_platform.hpp"
#include "../CPP_class/class_nullptr.hpp"

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

int32_t cmp_cma_get_page_size(ft_size_t *page_size_out)
{
    SYSTEM_INFO system_info;

    if (page_size_out == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    GetSystemInfo(&system_info);
    if (system_info.dwPageSize == 0)
        return (FT_ERR_INVALID_STATE);
    *page_size_out = static_cast<ft_size_t>(system_info.dwPageSize);
    return (FT_ERR_SUCCESS);
}

void *cmp_cma_memory_map_read_write(ft_size_t mapping_size)
{
    void *memory_pointer;

    if (mapping_size == 0)
        return (ft_nullptr);
    memory_pointer = VirtualAlloc(ft_nullptr, mapping_size,
            MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return (memory_pointer);
}

int32_t cmp_cma_memory_protect_none(void *memory_pointer, ft_size_t mapping_size)
{
    DWORD old_protection;

    if (memory_pointer == ft_nullptr || mapping_size == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    if (VirtualProtect(memory_pointer, mapping_size, PAGE_NOACCESS,
            &old_protection) == 0)
        return (FT_ERR_INVALID_STATE);
    return (FT_ERR_SUCCESS);
}

int32_t cmp_cma_memory_protect_read_write(void *memory_pointer,
    ft_size_t mapping_size)
{
    DWORD old_protection;

    if (memory_pointer == ft_nullptr || mapping_size == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    if (VirtualProtect(memory_pointer, mapping_size, PAGE_READWRITE,
            &old_protection) == 0)
        return (FT_ERR_INVALID_STATE);
    return (FT_ERR_SUCCESS);
}

int32_t cmp_cma_memory_unmap(void *memory_pointer, ft_size_t mapping_size)
{
    (void)mapping_size;
    if (memory_pointer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (VirtualFree(memory_pointer, 0, MEM_RELEASE) == 0)
        return (FT_ERR_INVALID_STATE);
    return (FT_ERR_SUCCESS);
}

#else

#include <sys/mman.h>
#include <unistd.h>

int32_t cmp_cma_get_page_size(ft_size_t *page_size_out)
{
    int64_t system_page_size;

    if (page_size_out == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    system_page_size = sysconf(_SC_PAGESIZE);
    if (system_page_size <= 0)
        return (FT_ERR_INVALID_STATE);
    *page_size_out = static_cast<ft_size_t>(system_page_size);
    return (FT_ERR_SUCCESS);
}

void *cmp_cma_memory_map_read_write(ft_size_t mapping_size)
{
    void *memory_pointer;

    if (mapping_size == 0)
        return (ft_nullptr);
    memory_pointer = mmap(ft_nullptr, mapping_size, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory_pointer == MAP_FAILED)
        return (ft_nullptr);
    return (memory_pointer);
}

int32_t cmp_cma_memory_protect_none(void *memory_pointer, ft_size_t mapping_size)
{
    if (memory_pointer == ft_nullptr || mapping_size == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    if (mprotect(memory_pointer, mapping_size, PROT_NONE) != 0)
        return (FT_ERR_INVALID_STATE);
    return (FT_ERR_SUCCESS);
}

int32_t cmp_cma_memory_protect_read_write(void *memory_pointer,
    ft_size_t mapping_size)
{
    if (memory_pointer == ft_nullptr || mapping_size == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    if (mprotect(memory_pointer, mapping_size, PROT_READ | PROT_WRITE) != 0)
        return (FT_ERR_INVALID_STATE);
    return (FT_ERR_SUCCESS);
}

int32_t cmp_cma_memory_unmap(void *memory_pointer, ft_size_t mapping_size)
{
    if (memory_pointer == ft_nullptr || mapping_size == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    if (munmap(memory_pointer, mapping_size) != 0)
        return (FT_ERR_INVALID_STATE);
    return (FT_ERR_SUCCESS);
}

#endif
