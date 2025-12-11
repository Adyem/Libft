#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "readline_internal.hpp"
#include "readline.hpp"

void rl_add_suggestion(const char *word)
{
    int index = 0;
    while (index < suggestion_count)
    {
        if (strcmp(suggestions[index], word) == 0)
        {
            ft_errno = FT_ERR_SUCCESSS;
            return ;
        }
        index++;
    }
    if (suggestion_count < MAX_SUGGESTIONS)
    {
        char *new_suggestion;

        new_suggestion = cma_strdup(word);
        if (new_suggestion == ft_nullptr)
            return ;
        suggestions[suggestion_count] = new_suggestion;
        suggestion_count++;
        ft_errno = FT_ERR_SUCCESSS;
    }
    else
    {
        pf_printf_fd(2, "Suggestion list full\n");
        ft_errno = FT_ERR_OUT_OF_RANGE;
    }
    return ;
}

void rl_clear_suggestions()
{
    int index = 0;
    while (index < suggestion_count)
    {
        cma_free(suggestions[index]);
        index++;
    }
    suggestion_count = 0;
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}
