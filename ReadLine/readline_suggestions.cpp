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
    int error_code;
    int index = 0;
    while (index < suggestion_count)
    {
        if (strcmp(suggestions[index], word) == 0)
        {
            error_code = FT_ERR_SUCCESSS;
            ft_global_error_stack_push(error_code);
            return ;
        }
        index++;
    }
    if (suggestion_count < MAX_SUGGESTIONS)
    {
        char *new_suggestion;

        new_suggestion = cma_strdup(word);
        if (new_suggestion == ft_nullptr)
        {
            error_code = ft_global_error_stack_pop_newest();
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_NO_MEMORY;
            ft_global_error_stack_push(error_code);
            return ;
        }
        suggestions[suggestion_count] = new_suggestion;
        suggestion_count++;
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
    }
    else
    {
        pf_printf_fd(2, "Suggestion list full\n");
        error_code = FT_ERR_OUT_OF_RANGE;
        ft_global_error_stack_push(error_code);
    }
    return ;
}

void rl_clear_suggestions()
{
    int error_code;
    int index = 0;
    while (index < suggestion_count)
    {
        cma_free(suggestions[index]);
        error_code = ft_global_error_stack_pop_newest();
        if (error_code != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(error_code);
            suggestion_count = 0;
            return ;
        }
        index++;
    }
    suggestion_count = 0;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return ;
}
