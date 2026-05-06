#ifndef MATH_INTERNAL_HPP
# define MATH_INTERNAL_HPP

#include "roll.hpp"

int32_t     math_roll_check_number_next(char *string, int32_t index);
int32_t     math_roll_check_number_previous(char *string, int32_t index);
int32_t     math_roll_check_character(char character);
int32_t     math_check_string_number(char *string);

void    math_free_parse(char **parsed_strings);
int32_t     math_roll_convert_previous(char *string, int32_t *index,
                int32_t *error);
int32_t     math_roll_convert_next(char *string, int32_t index,
                int32_t *error);
int32_t     math_roll_itoa(int32_t result, int32_t *index, char *string);
int32_t     math_is_unary_sign(const char *string, int32_t index);

void    math_calculate_j(char *string, int32_t *string_boundary);
int32_t     math_roll_validate(char *string);
int32_t     math_roll_parse_brackets(char *string);
int32_t     math_roll_excecute_droll(char *string, int32_t *index,
                int32_t string_boundary);
int32_t     math_roll_excecute_md(char *string, int32_t *index,
                int32_t string_boundary);
int32_t     math_roll_excecute_pm(char *string, int32_t *index,
                int32_t string_boundary);
int32_t     math_process_sign(char *string, int32_t *index,
                int32_t string_boundary, int32_t *error);

#endif
