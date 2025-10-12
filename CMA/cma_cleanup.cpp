#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdlib>

extern Page *page_list;

void cma_cleanup()
{
    if (OFFSWITCH)
        return ;
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
        return ;
    bool leak_detected = false;

    if (g_cma_current_bytes != 0 || g_cma_allocation_count != g_cma_free_count)
        leak_detected = true;
    Page* current_page = page_list;
    while (current_page)
    {
        Page* next_page = current_page->next;
        if (current_page->start && current_page->heap == true)
        {
            std::free(current_page->start);
        }
        cma_metadata_release_block(current_page->blocks);
        std::free(current_page);
        current_page = next_page;
    }
    page_list = ft_nullptr;
    g_cma_current_bytes = 0;
    if (leak_detected)
        g_cma_free_count = g_cma_allocation_count;
    cma_metadata_reset();
    allocator_guard.unlock();
    ft_errno = ER_SUCCESS;
    return ;
}
