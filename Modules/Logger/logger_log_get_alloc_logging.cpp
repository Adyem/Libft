#include "logger_internal.hpp"
#include "../CMA/CMA.hpp"

ft_bool ft_log_get_alloc_logging()
{
    return (cma_get_alloc_logging());
}
