#ifndef MATH_INTERNAL_HPP
# define MATH_INTERNAL_HPP

#include "roll.hpp"

int     math_roll_check_number_next(char *string, int index);
int     math_roll_check_number_previous(char *string, int index);
int     math_roll_check_character(char character);
int     math_check_string_number(char *string);

void    math_free_parse(char **parsed_strings);
int     math_roll_convert_previous(char *string, int *index, int *error);
int     math_roll_convert_next(char *string, int index, int *error);
int     math_roll_itoa(int result, int *index, char *string);

void    math_calculate_j(char *string, int *string_boundary);
int     math_roll_validate(char *string);
int     math_roll_parse_brackets(char *string);
int     math_roll_excecute_droll(char *string, int *index, int string_boundary);
int     math_roll_excecute_md(char *string, int *index, int string_boundary);
int     math_roll_excecute_pm(char *string, int *index, int string_boundary);
int     math_process_sign(char *string, int *index, int string_boundary, int *error);

#endif
