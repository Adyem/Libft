#include "readline.hpp"
#include "readline_internal.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

void rl_clear_history()
{
    int32_t index;

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
