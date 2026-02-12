#include "printf_internal.hpp"
#include "../Basic/basic.hpp"
#include "../CPP_class/class_string.hpp"
#include <cstdarg>
#include <unistd.h>
#include "../System_utils/system_utils.hpp"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <cstdio>

int pf_string_pop_last_error(const ft_string &string_value)
{
    unsigned long long operation_id;

    operation_id = string_value.last_operation_id();
    if (operation_id == 0)
        return (FT_ERR_SUCCESS);
    return (string_value.pop_operation_error(operation_id));
}

static const char g_decimal_pairs[200] =
{
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
    '1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
    '2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
    '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
    '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
    '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
    '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
    '8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
    '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'
};

static const char g_hex_pairs_lowercase[512] =
{
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
    '0', 'a', '0', 'b', '0', 'c', '0', 'd', '0', 'e', '0', 'f', '1', '0', '1', '1', '1', '2', '1', '3',
    '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '1', 'a', '1', 'b', '1', 'c', '1', 'd',
    '1', 'e', '1', 'f', '2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7',
    '2', '8', '2', '9', '2', 'a', '2', 'b', '2', 'c', '2', 'd', '2', 'e', '2', 'f', '3', '0', '3', '1',
    '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9', '3', 'a', '3', 'b',
    '3', 'c', '3', 'd', '3', 'e', '3', 'f', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5',
    '4', '6', '4', '7', '4', '8', '4', '9', '4', 'a', '4', 'b', '4', 'c', '4', 'd', '4', 'e', '4', 'f',
    '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '5', 'a', '5', 'b', '5', 'c', '5', 'd', '5', 'e', '5', 'f', '6', '0', '6', '1', '6', '2', '6', '3',
    '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9', '6', 'a', '6', 'b', '6', 'c', '6', 'd',
    '6', 'e', '6', 'f', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7',
    '7', '8', '7', '9', '7', 'a', '7', 'b', '7', 'c', '7', 'd', '7', 'e', '7', 'f', '8', '0', '8', '1',
    '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9', '8', 'a', '8', 'b',
    '8', 'c', '8', 'd', '8', 'e', '8', 'f', '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5',
    '9', '6', '9', '7', '9', '8', '9', '9', '9', 'a', '9', 'b', '9', 'c', '9', 'd', '9', 'e', '9', 'f',
    'a', '0', 'a', '1', 'a', '2', 'a', '3', 'a', '4', 'a', '5', 'a', '6', 'a', '7', 'a', '8', 'a', '9',
    'a', 'a', 'a', 'b', 'a', 'c', 'a', 'd', 'a', 'e', 'a', 'f', 'b', '0', 'b', '1', 'b', '2', 'b', '3',
    'b', '4', 'b', '5', 'b', '6', 'b', '7', 'b', '8', 'b', '9', 'b', 'a', 'b', 'b', 'b', 'c', 'b', 'd',
    'b', 'e', 'b', 'f', 'c', '0', 'c', '1', 'c', '2', 'c', '3', 'c', '4', 'c', '5', 'c', '6', 'c', '7',
    'c', '8', 'c', '9', 'c', 'a', 'c', 'b', 'c', 'c', 'c', 'd', 'c', 'e', 'c', 'f', 'd', '0', 'd', '1',
    'd', '2', 'd', '3', 'd', '4', 'd', '5', 'd', '6', 'd', '7', 'd', '8', 'd', '9', 'd', 'a', 'd', 'b',
    'd', 'c', 'd', 'd', 'd', 'e', 'd', 'f', 'e', '0', 'e', '1', 'e', '2', 'e', '3', 'e', '4', 'e', '5',
    'e', '6', 'e', '7', 'e', '8', 'e', '9', 'e', 'a', 'e', 'b', 'e', 'c', 'e', 'd', 'e', 'e', 'e', 'f',
    'f', '0', 'f', '1', 'f', '2', 'f', '3', 'f', '4', 'f', '5', 'f', '6', 'f', '7', 'f', '8', 'f', '9',
    'f', 'a', 'f', 'b', 'f', 'c', 'f', 'd', 'f', 'e', 'f', 'f'
};

static const char g_hex_pairs_uppercase[512] =
{
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
    '0', 'A', '0', 'B', '0', 'C', '0', 'D', '0', 'E', '0', 'F', '1', '0', '1', '1', '1', '2', '1', '3',
    '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '1', 'A', '1', 'B', '1', 'C', '1', 'D',
    '1', 'E', '1', 'F', '2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7',
    '2', '8', '2', '9', '2', 'A', '2', 'B', '2', 'C', '2', 'D', '2', 'E', '2', 'F', '3', '0', '3', '1',
    '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9', '3', 'A', '3', 'B',
    '3', 'C', '3', 'D', '3', 'E', '3', 'F', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5',
    '4', '6', '4', '7', '4', '8', '4', '9', '4', 'A', '4', 'B', '4', 'C', '4', 'D', '4', 'E', '4', 'F',
    '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
    '5', 'A', '5', 'B', '5', 'C', '5', 'D', '5', 'E', '5', 'F', '6', '0', '6', '1', '6', '2', '6', '3',
    '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9', '6', 'A', '6', 'B', '6', 'C', '6', 'D',
    '6', 'E', '6', 'F', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7',
    '7', '8', '7', '9', '7', 'A', '7', 'B', '7', 'C', '7', 'D', '7', 'E', '7', 'F', '8', '0', '8', '1',
    '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9', '8', 'A', '8', 'B',
    '8', 'C', '8', 'D', '8', 'E', '8', 'F', '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5',
    '9', '6', '9', '7', '9', '8', '9', '9', '9', 'A', '9', 'B', '9', 'C', '9', 'D', '9', 'E', '9', 'F',
    'A', '0', 'A', '1', 'A', '2', 'A', '3', 'A', '4', 'A', '5', 'A', '6', 'A', '7', 'A', '8', 'A', '9',
    'A', 'A', 'A', 'B', 'A', 'C', 'A', 'D', 'A', 'E', 'A', 'F', 'B', '0', 'B', '1', 'B', '2', 'B', '3',
    'B', '4', 'B', '5', 'B', '6', 'B', '7', 'B', '8', 'B', '9', 'B', 'A', 'B', 'B', 'B', 'C', 'B', 'D',
    'B', 'E', 'B', 'F', 'C', '0', 'C', '1', 'C', '2', 'C', '3', 'C', '4', 'C', '5', 'C', '6', 'C', '7',
    'C', '8', 'C', '9', 'C', 'A', 'C', 'B', 'C', 'C', 'C', 'D', 'C', 'E', 'C', 'F', 'D', '0', 'D', '1',
    'D', '2', 'D', '3', 'D', '4', 'D', '5', 'D', '6', 'D', '7', 'D', '8', 'D', '9', 'D', 'A', 'D', 'B',
    'D', 'C', 'D', 'D', 'D', 'E', 'D', 'F', 'E', '0', 'E', '1', 'E', '2', 'E', '3', 'E', '4', 'E', '5',
    'E', '6', 'E', '7', 'E', '8', 'E', '9', 'E', 'A', 'E', 'B', 'E', 'C', 'E', 'D', 'E', 'E', 'E', 'F',
    'F', '0', 'F', '1', 'F', '2', 'F', '3', 'F', '4', 'F', '5', 'F', '6', 'F', '7', 'F', '8', 'F', '9',
    'F', 'A', 'F', 'B', 'F', 'C', 'F', 'D', 'F', 'E', 'F', 'F'
};

static const char g_octal_triplets[512][3] =
{
    { '0', '0', '0' }, { '0', '0', '1' }, { '0', '0', '2' }, { '0', '0', '3' },
    { '0', '0', '4' }, { '0', '0', '5' }, { '0', '0', '6' }, { '0', '0', '7' },
    { '0', '1', '0' }, { '0', '1', '1' }, { '0', '1', '2' }, { '0', '1', '3' },
    { '0', '1', '4' }, { '0', '1', '5' }, { '0', '1', '6' }, { '0', '1', '7' },
    { '0', '2', '0' }, { '0', '2', '1' }, { '0', '2', '2' }, { '0', '2', '3' },
    { '0', '2', '4' }, { '0', '2', '5' }, { '0', '2', '6' }, { '0', '2', '7' },
    { '0', '3', '0' }, { '0', '3', '1' }, { '0', '3', '2' }, { '0', '3', '3' },
    { '0', '3', '4' }, { '0', '3', '5' }, { '0', '3', '6' }, { '0', '3', '7' },
    { '0', '4', '0' }, { '0', '4', '1' }, { '0', '4', '2' }, { '0', '4', '3' },
    { '0', '4', '4' }, { '0', '4', '5' }, { '0', '4', '6' }, { '0', '4', '7' },
    { '0', '5', '0' }, { '0', '5', '1' }, { '0', '5', '2' }, { '0', '5', '3' },
    { '0', '5', '4' }, { '0', '5', '5' }, { '0', '5', '6' }, { '0', '5', '7' },
    { '0', '6', '0' }, { '0', '6', '1' }, { '0', '6', '2' }, { '0', '6', '3' },
    { '0', '6', '4' }, { '0', '6', '5' }, { '0', '6', '6' }, { '0', '6', '7' },
    { '0', '7', '0' }, { '0', '7', '1' }, { '0', '7', '2' }, { '0', '7', '3' },
    { '0', '7', '4' }, { '0', '7', '5' }, { '0', '7', '6' }, { '0', '7', '7' },
    { '1', '0', '0' }, { '1', '0', '1' }, { '1', '0', '2' }, { '1', '0', '3' },
    { '1', '0', '4' }, { '1', '0', '5' }, { '1', '0', '6' }, { '1', '0', '7' },
    { '1', '1', '0' }, { '1', '1', '1' }, { '1', '1', '2' }, { '1', '1', '3' },
    { '1', '1', '4' }, { '1', '1', '5' }, { '1', '1', '6' }, { '1', '1', '7' },
    { '1', '2', '0' }, { '1', '2', '1' }, { '1', '2', '2' }, { '1', '2', '3' },
    { '1', '2', '4' }, { '1', '2', '5' }, { '1', '2', '6' }, { '1', '2', '7' },
    { '1', '3', '0' }, { '1', '3', '1' }, { '1', '3', '2' }, { '1', '3', '3' },
    { '1', '3', '4' }, { '1', '3', '5' }, { '1', '3', '6' }, { '1', '3', '7' },
    { '1', '4', '0' }, { '1', '4', '1' }, { '1', '4', '2' }, { '1', '4', '3' },
    { '1', '4', '4' }, { '1', '4', '5' }, { '1', '4', '6' }, { '1', '4', '7' },
    { '1', '5', '0' }, { '1', '5', '1' }, { '1', '5', '2' }, { '1', '5', '3' },
    { '1', '5', '4' }, { '1', '5', '5' }, { '1', '5', '6' }, { '1', '5', '7' },
    { '1', '6', '0' }, { '1', '6', '1' }, { '1', '6', '2' }, { '1', '6', '3' },
    { '1', '6', '4' }, { '1', '6', '5' }, { '1', '6', '6' }, { '1', '6', '7' },
    { '1', '7', '0' }, { '1', '7', '1' }, { '1', '7', '2' }, { '1', '7', '3' },
    { '1', '7', '4' }, { '1', '7', '5' }, { '1', '7', '6' }, { '1', '7', '7' },
    { '2', '0', '0' }, { '2', '0', '1' }, { '2', '0', '2' }, { '2', '0', '3' },
    { '2', '0', '4' }, { '2', '0', '5' }, { '2', '0', '6' }, { '2', '0', '7' },
    { '2', '1', '0' }, { '2', '1', '1' }, { '2', '1', '2' }, { '2', '1', '3' },
    { '2', '1', '4' }, { '2', '1', '5' }, { '2', '1', '6' }, { '2', '1', '7' },
    { '2', '2', '0' }, { '2', '2', '1' }, { '2', '2', '2' }, { '2', '2', '3' },
    { '2', '2', '4' }, { '2', '2', '5' }, { '2', '2', '6' }, { '2', '2', '7' },
    { '2', '3', '0' }, { '2', '3', '1' }, { '2', '3', '2' }, { '2', '3', '3' },
    { '2', '3', '4' }, { '2', '3', '5' }, { '2', '3', '6' }, { '2', '3', '7' },
    { '2', '4', '0' }, { '2', '4', '1' }, { '2', '4', '2' }, { '2', '4', '3' },
    { '2', '4', '4' }, { '2', '4', '5' }, { '2', '4', '6' }, { '2', '4', '7' },
    { '2', '5', '0' }, { '2', '5', '1' }, { '2', '5', '2' }, { '2', '5', '3' },
    { '2', '5', '4' }, { '2', '5', '5' }, { '2', '5', '6' }, { '2', '5', '7' },
    { '2', '6', '0' }, { '2', '6', '1' }, { '2', '6', '2' }, { '2', '6', '3' },
    { '2', '6', '4' }, { '2', '6', '5' }, { '2', '6', '6' }, { '2', '6', '7' },
    { '2', '7', '0' }, { '2', '7', '1' }, { '2', '7', '2' }, { '2', '7', '3' },
    { '2', '7', '4' }, { '2', '7', '5' }, { '2', '7', '6' }, { '2', '7', '7' },
    { '3', '0', '0' }, { '3', '0', '1' }, { '3', '0', '2' }, { '3', '0', '3' },
    { '3', '0', '4' }, { '3', '0', '5' }, { '3', '0', '6' }, { '3', '0', '7' },
    { '3', '1', '0' }, { '3', '1', '1' }, { '3', '1', '2' }, { '3', '1', '3' },
    { '3', '1', '4' }, { '3', '1', '5' }, { '3', '1', '6' }, { '3', '1', '7' },
    { '3', '2', '0' }, { '3', '2', '1' }, { '3', '2', '2' }, { '3', '2', '3' },
    { '3', '2', '4' }, { '3', '2', '5' }, { '3', '2', '6' }, { '3', '2', '7' },
    { '3', '3', '0' }, { '3', '3', '1' }, { '3', '3', '2' }, { '3', '3', '3' },
    { '3', '3', '4' }, { '3', '3', '5' }, { '3', '3', '6' }, { '3', '3', '7' },
    { '3', '4', '0' }, { '3', '4', '1' }, { '3', '4', '2' }, { '3', '4', '3' },
    { '3', '4', '4' }, { '3', '4', '5' }, { '3', '4', '6' }, { '3', '4', '7' },
    { '3', '5', '0' }, { '3', '5', '1' }, { '3', '5', '2' }, { '3', '5', '3' },
    { '3', '5', '4' }, { '3', '5', '5' }, { '3', '5', '6' }, { '3', '5', '7' },
    { '3', '6', '0' }, { '3', '6', '1' }, { '3', '6', '2' }, { '3', '6', '3' },
    { '3', '6', '4' }, { '3', '6', '5' }, { '3', '6', '6' }, { '3', '6', '7' },
    { '3', '7', '0' }, { '3', '7', '1' }, { '3', '7', '2' }, { '3', '7', '3' },
    { '3', '7', '4' }, { '3', '7', '5' }, { '3', '7', '6' }, { '3', '7', '7' },
    { '4', '0', '0' }, { '4', '0', '1' }, { '4', '0', '2' }, { '4', '0', '3' },
    { '4', '0', '4' }, { '4', '0', '5' }, { '4', '0', '6' }, { '4', '0', '7' },
    { '4', '1', '0' }, { '4', '1', '1' }, { '4', '1', '2' }, { '4', '1', '3' },
    { '4', '1', '4' }, { '4', '1', '5' }, { '4', '1', '6' }, { '4', '1', '7' },
    { '4', '2', '0' }, { '4', '2', '1' }, { '4', '2', '2' }, { '4', '2', '3' },
    { '4', '2', '4' }, { '4', '2', '5' }, { '4', '2', '6' }, { '4', '2', '7' },
    { '4', '3', '0' }, { '4', '3', '1' }, { '4', '3', '2' }, { '4', '3', '3' },
    { '4', '3', '4' }, { '4', '3', '5' }, { '4', '3', '6' }, { '4', '3', '7' },
    { '4', '4', '0' }, { '4', '4', '1' }, { '4', '4', '2' }, { '4', '4', '3' },
    { '4', '4', '4' }, { '4', '4', '5' }, { '4', '4', '6' }, { '4', '4', '7' },
    { '4', '5', '0' }, { '4', '5', '1' }, { '4', '5', '2' }, { '4', '5', '3' },
    { '4', '5', '4' }, { '4', '5', '5' }, { '4', '5', '6' }, { '4', '5', '7' },
    { '4', '6', '0' }, { '4', '6', '1' }, { '4', '6', '2' }, { '4', '6', '3' },
    { '4', '6', '4' }, { '4', '6', '5' }, { '4', '6', '6' }, { '4', '6', '7' },
    { '4', '7', '0' }, { '4', '7', '1' }, { '4', '7', '2' }, { '4', '7', '3' },
    { '4', '7', '4' }, { '4', '7', '5' }, { '4', '7', '6' }, { '4', '7', '7' },
    { '5', '0', '0' }, { '5', '0', '1' }, { '5', '0', '2' }, { '5', '0', '3' },
    { '5', '0', '4' }, { '5', '0', '5' }, { '5', '0', '6' }, { '5', '0', '7' },
    { '5', '1', '0' }, { '5', '1', '1' }, { '5', '1', '2' }, { '5', '1', '3' },
    { '5', '1', '4' }, { '5', '1', '5' }, { '5', '1', '6' }, { '5', '1', '7' },
    { '5', '2', '0' }, { '5', '2', '1' }, { '5', '2', '2' }, { '5', '2', '3' },
    { '5', '2', '4' }, { '5', '2', '5' }, { '5', '2', '6' }, { '5', '2', '7' },
    { '5', '3', '0' }, { '5', '3', '1' }, { '5', '3', '2' }, { '5', '3', '3' },
    { '5', '3', '4' }, { '5', '3', '5' }, { '5', '3', '6' }, { '5', '3', '7' },
    { '5', '4', '0' }, { '5', '4', '1' }, { '5', '4', '2' }, { '5', '4', '3' },
    { '5', '4', '4' }, { '5', '4', '5' }, { '5', '4', '6' }, { '5', '4', '7' },
    { '5', '5', '0' }, { '5', '5', '1' }, { '5', '5', '2' }, { '5', '5', '3' },
    { '5', '5', '4' }, { '5', '5', '5' }, { '5', '5', '6' }, { '5', '5', '7' },
    { '5', '6', '0' }, { '5', '6', '1' }, { '5', '6', '2' }, { '5', '6', '3' },
    { '5', '6', '4' }, { '5', '6', '5' }, { '5', '6', '6' }, { '5', '6', '7' },
    { '5', '7', '0' }, { '5', '7', '1' }, { '5', '7', '2' }, { '5', '7', '3' },
    { '5', '7', '4' }, { '5', '7', '5' }, { '5', '7', '6' }, { '5', '7', '7' },
    { '6', '0', '0' }, { '6', '0', '1' }, { '6', '0', '2' }, { '6', '0', '3' },
    { '6', '0', '4' }, { '6', '0', '5' }, { '6', '0', '6' }, { '6', '0', '7' },
    { '6', '1', '0' }, { '6', '1', '1' }, { '6', '1', '2' }, { '6', '1', '3' },
    { '6', '1', '4' }, { '6', '1', '5' }, { '6', '1', '6' }, { '6', '1', '7' },
    { '6', '2', '0' }, { '6', '2', '1' }, { '6', '2', '2' }, { '6', '2', '3' },
    { '6', '2', '4' }, { '6', '2', '5' }, { '6', '2', '6' }, { '6', '2', '7' },
    { '6', '3', '0' }, { '6', '3', '1' }, { '6', '3', '2' }, { '6', '3', '3' },
    { '6', '3', '4' }, { '6', '3', '5' }, { '6', '3', '6' }, { '6', '3', '7' },
    { '6', '4', '0' }, { '6', '4', '1' }, { '6', '4', '2' }, { '6', '4', '3' },
    { '6', '4', '4' }, { '6', '4', '5' }, { '6', '4', '6' }, { '6', '4', '7' },
    { '6', '5', '0' }, { '6', '5', '1' }, { '6', '5', '2' }, { '6', '5', '3' },
    { '6', '5', '4' }, { '6', '5', '5' }, { '6', '5', '6' }, { '6', '5', '7' },
    { '6', '6', '0' }, { '6', '6', '1' }, { '6', '6', '2' }, { '6', '6', '3' },
    { '6', '6', '4' }, { '6', '6', '5' }, { '6', '6', '6' }, { '6', '6', '7' },
    { '6', '7', '0' }, { '6', '7', '1' }, { '6', '7', '2' }, { '6', '7', '3' },
    { '6', '7', '4' }, { '6', '7', '5' }, { '6', '7', '6' }, { '6', '7', '7' },
    { '7', '0', '0' }, { '7', '0', '1' }, { '7', '0', '2' }, { '7', '0', '3' },
    { '7', '0', '4' }, { '7', '0', '5' }, { '7', '0', '6' }, { '7', '0', '7' },
    { '7', '1', '0' }, { '7', '1', '1' }, { '7', '1', '2' }, { '7', '1', '3' },
    { '7', '1', '4' }, { '7', '1', '5' }, { '7', '1', '6' }, { '7', '1', '7' },
    { '7', '2', '0' }, { '7', '2', '1' }, { '7', '2', '2' }, { '7', '2', '3' },
    { '7', '2', '4' }, { '7', '2', '5' }, { '7', '2', '6' }, { '7', '2', '7' },
    { '7', '3', '0' }, { '7', '3', '1' }, { '7', '3', '2' }, { '7', '3', '3' },
    { '7', '3', '4' }, { '7', '3', '5' }, { '7', '3', '6' }, { '7', '3', '7' },
    { '7', '4', '0' }, { '7', '4', '1' }, { '7', '4', '2' }, { '7', '4', '3' },
    { '7', '4', '4' }, { '7', '4', '5' }, { '7', '4', '6' }, { '7', '4', '7' },
    { '7', '5', '0' }, { '7', '5', '1' }, { '7', '5', '2' }, { '7', '5', '3' },
    { '7', '5', '4' }, { '7', '5', '5' }, { '7', '5', '6' }, { '7', '5', '7' },
    { '7', '6', '0' }, { '7', '6', '1' }, { '7', '6', '2' }, { '7', '6', '3' },
    { '7', '6', '4' }, { '7', '6', '5' }, { '7', '6', '6' }, { '7', '6', '7' },
    { '7', '7', '0' }, { '7', '7', '1' }, { '7', '7', '2' }, { '7', '7', '3' },
    { '7', '7', '4' }, { '7', '7', '5' }, { '7', '7', '6' }, { '7', '7', '7' }
};

static bool count_has_error(size_t *count)
{
    if (!count)
        return (true);
    if (*count == SIZE_MAX)
        return (true);
    return (false);
}

static void mark_count_error(size_t *count)
{
    if (!count)
        return ;
    *count = SIZE_MAX;
    return ;
}

static void write_buffer_fd(const char *buffer, size_t length, int fd, size_t *count)
{
    if (count_has_error(count))
        return ;
    if (length == 0)
        return ;
    ssize_t return_value = su_write(fd, buffer, length);
    if (return_value != static_cast<ssize_t>(length))
    {
        mark_count_error(count);
        return ;
    }
    *count += length;
    return ;
}

static void write_decimal_from_pairs(uintmax_t number, int fd, size_t *count)
{
    char        buffer[32];
    char        *cursor;

    if (count_has_error(count))
        return ;
    cursor = buffer + sizeof(buffer);
    while (number >= 100)
    {
        uintmax_t chunk = number % 100;
        size_t pair_index = chunk * 2;

        number /= 100;
        cursor -= 2;
        cursor[0] = g_decimal_pairs[pair_index];
        cursor[1] = g_decimal_pairs[pair_index + 1];
    }
    if (number < 10)
    {
        cursor -= 1;
        cursor[0] = static_cast<char>('0' + number);
    }
    else
    {
        size_t pair_index = number * 2;

        cursor -= 2;
        cursor[0] = g_decimal_pairs[pair_index];
        cursor[1] = g_decimal_pairs[pair_index + 1];
    }
    size_t length = static_cast<size_t>((buffer + sizeof(buffer)) - cursor);

    write_buffer_fd(cursor, length, fd, count);
    return ;
}

static void write_hex_from_pairs(uintmax_t number, bool uppercase, int fd, size_t *count)
{
    char        buffer[32];
    char        *cursor;

    if (count_has_error(count))
        return ;
    cursor = buffer + sizeof(buffer);
    if (number == 0)
    {
        cursor -= 1;
        cursor[0] = '0';
        write_buffer_fd(cursor, 1, fd, count);
        return ;
    }
    while (number != 0)
    {
        unsigned int byte_value = static_cast<unsigned int>(number & 0xFF);
        size_t pair_index = static_cast<size_t>(byte_value) * 2;

        number >>= 8;
        cursor -= 2;
        if (uppercase)
        {
            cursor[0] = g_hex_pairs_uppercase[pair_index];
            cursor[1] = g_hex_pairs_uppercase[pair_index + 1];
        }
        else
        {
            cursor[0] = g_hex_pairs_lowercase[pair_index];
            cursor[1] = g_hex_pairs_lowercase[pair_index + 1];
        }
    }
    if (cursor[0] == '0')
    {
        cursor += 1;
    }
    size_t length = static_cast<size_t>((buffer + sizeof(buffer)) - cursor);

    write_buffer_fd(cursor, length, fd, count);
    return ;
}

static void write_octal_from_triplets(uintmax_t number, int fd, size_t *count)
{
    char        buffer[66];
    char        *cursor;

    if (count_has_error(count))
        return ;
    cursor = buffer + sizeof(buffer);
    if (number == 0)
    {
        cursor -= 1;
        cursor[0] = '0';
        write_buffer_fd(cursor, 1, fd, count);
        return ;
    }
    while (number != 0)
    {
        unsigned int block_value = static_cast<unsigned int>(number & 0x1FF);
        const char *triplet = g_octal_triplets[block_value];

        number >>= 9;
        cursor -= 3;
        cursor[0] = triplet[0];
        cursor[1] = triplet[1];
        cursor[2] = triplet[2];
    }
    while (cursor[0] == '0' && static_cast<size_t>((buffer + sizeof(buffer)) - cursor) > 1)
    {
        cursor += 1;
    }
    size_t length = static_cast<size_t>((buffer + sizeof(buffer)) - cursor);

    write_buffer_fd(cursor, length, fd, count);
    return ;
}

static int format_double_output(char specifier, int precision, double number, ft_string &output)
{
    if (precision < 0)
        precision = 6;
    if ((specifier == 'g' || specifier == 'G') && precision == 0)
        precision = 1;
#define FORMAT_DOUBLE_CASE(character, literal) \
    if (specifier == character) \
    { \
        int required_length = std::snprintf(ft_nullptr, 0, literal, precision, number); \
        char *output_buffer = ft_nullptr; \
        if (required_length < 0) \
            return (-1); \
        output.clear(); \
        if (pf_string_pop_last_error(output) != FT_ERR_SUCCESS) \
            return (-1); \
        output.resize_length(static_cast<size_t>(required_length)); \
        if (pf_string_pop_last_error(output) != FT_ERR_SUCCESS) \
            return (-1); \
        output_buffer = output.print(); \
        if (pf_string_pop_last_error(output) != FT_ERR_SUCCESS || output_buffer == ft_nullptr) \
            return (-1); \
        int written_length = std::snprintf(output_buffer, static_cast<size_t>(required_length) + 1, literal, precision, number); \
        if (written_length < 0) \
            return (-1); \
        output.resize_length(static_cast<size_t>(written_length)); \
        if (pf_string_pop_last_error(output) != FT_ERR_SUCCESS) \
            return (-1); \
        return (0); \
    }

    FORMAT_DOUBLE_CASE('f', "%.*f");
    FORMAT_DOUBLE_CASE('F', "%.*F");
    FORMAT_DOUBLE_CASE('e', "%.*e");
    FORMAT_DOUBLE_CASE('E', "%.*E");
    FORMAT_DOUBLE_CASE('g', "%.*g");
    FORMAT_DOUBLE_CASE('G', "%.*G");
#undef FORMAT_DOUBLE_CASE
    return (-1);
}

size_t ft_strlen_printf(const char *string)
{
    size_t length = 0;
    if (!string)
        return (6);
    while (string[length])
        length++;
    return (length);
}

void ft_putchar_fd(const char character, int fd, size_t *count)
{
    write_buffer_fd(&character, 1, fd, count);
    return ;
}

void ft_putstr_fd(const char *string, int fd, size_t *count)
{
    if (count_has_error(count))
        return ;
    if (!string)
    {
        write_buffer_fd("(null)", 6, fd, count);
        return ;
    }
    size_t length = ft_strlen_printf(string);
    write_buffer_fd(string, length, fd, count);
    return ;
}

void ft_putnbr_fd(long number, int fd, size_t *count)
{
    uintmax_t  absolute_value;

    if (count_has_error(count))
        return ;
    if (number < 0)
    {
        ft_putchar_fd('-', fd, count);
        if (count_has_error(count))
            return ;
        absolute_value = static_cast<uintmax_t>(-(number + 1)) + 1;
    }
    else
    {
        absolute_value = static_cast<uintmax_t>(number);
    }
    write_decimal_from_pairs(absolute_value, fd, count);
    return ;
}

void ft_putunsigned_fd(uintmax_t number, int fd, size_t *count)
{
    write_decimal_from_pairs(number, fd, count);
    return ;
}

void ft_puthex_fd(uintmax_t number, int fd, bool uppercase, size_t *count)
{
    write_hex_from_pairs(number, uppercase, fd, count);
    return ;
}

void ft_putoctal_fd(uintmax_t number, int fd, size_t *count)
{
    write_octal_from_triplets(number, fd, count);
    return ;
}

void ft_putptr_fd(void *pointer, int fd, size_t *count)
{
    if (count_has_error(count))
        return ;
    uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
    ft_putstr_fd("0x", fd, count);
    if (count_has_error(count))
        return ;
    ft_puthex_fd(address, fd, false, count);
    return ;
}

void ft_putfloat_fd(double number, int fd, size_t *count, int precision)
{
    if (count_has_error(count))
        return ;
    ft_string formatted_output;
    if (format_double_output('f', precision, number, formatted_output) != 0)
    {
        mark_count_error(count);
        return ;
    }
    size_t output_length = formatted_output.size();
    if (output_length == 0)
        return ;
    write_buffer_fd(formatted_output.c_str(), output_length, fd, count);
    return ;
}

void ft_putscientific_fd(double number, bool uppercase, int fd, size_t *count, int precision)
{
    if (count_has_error(count))
        return ;
    char specifier;

    if (uppercase)
        specifier = 'E';
    else
        specifier = 'e';
    ft_string formatted_output;
    if (format_double_output(specifier, precision, number, formatted_output) != 0)
    {
        mark_count_error(count);
        return ;
    }
    size_t output_length = formatted_output.size();
    if (output_length == 0)
        return ;
    write_buffer_fd(formatted_output.c_str(), output_length, fd, count);
    return ;
}

void ft_putgeneral_fd(double number, bool uppercase, int fd, size_t *count, int precision)
{
    if (count_has_error(count))
        return ;
    char specifier;

    if (uppercase)
        specifier = 'G';
    else
        specifier = 'g';
    ft_string formatted_output;
    if (format_double_output(specifier, precision, number, formatted_output) != 0)
    {
        mark_count_error(count);
        return ;
    }
    size_t output_length = formatted_output.size();
    if (output_length == 0)
        return ;
    write_buffer_fd(formatted_output.c_str(), output_length, fd, count);
    return ;
}

void pf_write_ft_string_fd(const ft_string &output, int fd, size_t *count)
{
    const char  *buffer;
    size_t      length;

    if (count_has_error(count))
        return ;
    if (pf_string_pop_last_error(output) != FT_ERR_SUCCESS)
    {
        mark_count_error(count);
        return ;
    }
    buffer = output.c_str();
    if (pf_string_pop_last_error(output) != FT_ERR_SUCCESS || buffer == ft_nullptr)
    {
        mark_count_error(count);
        return ;
    }
    length = output.size();
    if (pf_string_pop_last_error(output) != FT_ERR_SUCCESS)
    {
        mark_count_error(count);
        return ;
    }
    if (length == 0)
        return ;
    write_buffer_fd(buffer, length, fd, count);
    return ;
}
