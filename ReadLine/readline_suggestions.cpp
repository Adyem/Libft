#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../Advanced/advanced.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
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
            return ;
        index++;
    }
    if (suggestion_count < MAX_SUGGESTIONS)
    {
        char *new_suggestion;

        new_suggestion = adv_strdup(word);
        if (new_suggestion == ft_nullptr)
            return ;
        suggestions[suggestion_count] = new_suggestion;
        suggestion_count++;
    }
    else
    {
        pf_printf_fd(2, "Suggestion list full\n");
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
    return ;
}
