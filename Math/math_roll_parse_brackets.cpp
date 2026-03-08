#include "roll.hpp"
#include "math_internal.hpp"
#include "../Printf/printf.hpp"

int32_t math_roll_parse_brackets(char *string)
{
    int32_t read_index = 0;
    int32_t write_index = 0;
    int32_t bracket_found = 0;

    if (DEBUG == 1)
        pf_printf("Parsing brackets: %s\n", string);
    while (string[read_index])
    {
        if (string[read_index] == '(' && !bracket_found)
            bracket_found = 1;
        else if (string[read_index] == ')' && bracket_found)
            bracket_found = 0;
        else
            string[write_index++] = string[read_index];
        read_index++;
    }
    string[write_index] = '\0';
    if (DEBUG == 1)
        pf_printf("Modified string: %s\n", string);
    return (0);
}
