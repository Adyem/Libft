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
    {
        *terminal_width = 80;
        return (FT_ERR_SUCCESS);
    }
    if (rl_terminal_dimensions_refresh(&shared_dimensions) != FT_ERR_SUCCESS)
    {
        *terminal_width = 80;
        return (FT_ERR_SUCCESS);
    }
    if (rl_terminal_dimensions_get(&shared_dimensions, &row_count, &column_count, ft_nullptr,
        ft_nullptr, &dimensions_valid) != FT_ERR_SUCCESS)
    {
        *terminal_width = 80;
        return (FT_ERR_SUCCESS);
    }
    if (dimensions_valid == FT_FALSE)
    {
        *terminal_width = 80;
        return (FT_ERR_SUCCESS);
    }
    *terminal_width = static_cast<int32_t>(column_count);
    if (*terminal_width <= 0)
        *terminal_width = 80;
    return (FT_ERR_SUCCESS);
}
