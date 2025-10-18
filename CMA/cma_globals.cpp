#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"

Page *page_list = ft_nullptr;
ft_size_t    g_cma_alloc_limit = 0;
ft_size_t    g_cma_allocation_count = 0;
ft_size_t    g_cma_free_count = 0;
ft_size_t    g_cma_current_bytes = 0;
ft_size_t    g_cma_peak_bytes = 0;
unsigned long long    g_cma_metadata_access_depth = 0;

