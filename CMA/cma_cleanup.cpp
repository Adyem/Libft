#include "CMA.hpp"
#include "CMA_internal.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Printf/printf.hpp"
#include <cstdlib>
#include <new>

extern Page *page_list;

void cma_cleanup()
{
	if (DEBUG == 1)
		pf_printf("calling cleanup\n");
	if (OFFSWITCH)
		return ;
    Page* current_page = page_list;
    while (current_page)
    {
        Page* next_page = current_page->next;
        if (current_page->start && current_page->heap == true)
		{
			if (DEBUG == 1)
				pf_printf("freeing current page memory\n");
            ::operator delete(current_page->start, std::align_val_t(8), std::nothrow);
		}
		if (DEBUG == 1)
			pf_printf("Freeing current page metadata\n");
        ::operator delete(current_page, std::align_val_t(8), std::nothrow);
        current_page = next_page;
    }
    page_list = ft_nullptr;
	return ;
}
