#include "readline.hpp"
#include "readline_internal.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../CMA/CMA.hpp"

void rl_clear_history()
{
    int    index;

    index = 0;
    while (index < history_count)
    {
        cma_free(history[index]);
        history[index] = ft_nullptr;
        index++;
    }
    history_count = 0;
    return ;
}
