#include "readline.hpp"
#include "readline_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

void rl_clear_history()
{
    int    index;
    int    error_code;
    int    final_error;

    index = 0;
    final_error = FT_ERR_SUCCESS;
    while (index < history_count)
    {
        cma_free(history[index]);
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
            final_error = error_code;
        history[index] = ft_nullptr;
        index++;
    }
    history_count = 0;
    ft_global_error_stack_push(final_error);
    return ;
}
