#include "readline_internal.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t rl_get_terminal_width(int32_t *terminal_width)
{
    static terminal_dimensions shared_dimensions;
    uint16_t             row_count;
    uint16_t             column_count;
    ft_bool                       dimensions_valid;

    if (terminal_width == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (rl_terminal_dimensions_prepare_thread_safety(&shared_dimensions) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    if (rl_terminal_dimensions_refresh(&shared_dimensions) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    if (rl_terminal_dimensions_get(&shared_dimensions, &row_count, &column_count, ft_nullptr,
        ft_nullptr, &dimensions_valid) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    if (dimensions_valid == FT_FALSE)
        return (FT_ERR_INTERNAL);
    *terminal_width = static_cast<int32_t>(column_count);
    return (FT_ERR_SUCCESS);
}
