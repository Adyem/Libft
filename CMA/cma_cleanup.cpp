#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Printf/printf.hpp"
#include <cstdlib>

extern Page *page_list;

void cma_cleanup()
{
    if (DEBUG == 1)
        pf_printf("calling cleanup\n");
    if (OFFSWITCH)
        return ;
    bool lock_acquired;

    lock_acquired = false;
    if (cma_lock_allocator(&lock_acquired) != 0)
        return ;
    Page* current_page = page_list;
    while (current_page)
    {
        Page* next_page = current_page->next;
        if (current_page->start && current_page->heap == true)
        {
            if (DEBUG == 1)
                pf_printf("freeing current page memory\n");
            std::free(current_page->start);
        }
        if (DEBUG == 1)
            pf_printf("Freeing current page metadata\n");
        std::free(current_page);
        current_page = next_page;
    }
    page_list = ft_nullptr;
    g_cma_current_bytes = 0;
    cma_unlock_allocator(lock_acquired);
    return ;
}
