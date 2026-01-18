#include "printf_engine.hpp"
#include "printf_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include <vector>
#include <string>
#include <cctype>
#include <limits>
#include <cstdio>
#include <cwchar>
#include <cerrno>
#include <type_traits>

typedef typename std::make_signed<size_t>::type pf_signed_size_t;
typedef typename std::make_unsigned<ptrdiff_t>::type pf_unsigned_ptrdiff_t;

enum pf_length_modifier
{
    PF_LEN_NONE,
    PF_LEN_HH,
    PF_LEN_H,
    PF_LEN_L,
    PF_LEN_LL,
    PF_LEN_J,
    PF_LEN_Z,
    PF_LEN_T,
    PF_LEN_CAPITAL_L
};

struct pf_engine_format_spec
{
    bool flag_left_align;
    bool flag_force_sign;
    bool flag_space_sign;
    bool flag_alternate_form;
    bool flag_zero_pad;
    bool width_specified;
    bool width_from_argument;
    int width_value;
    int width_argument_index;
    bool precision_specified;
    bool precision_from_argument;
    int precision_value;
    int precision_argument_index;
    pf_length_modifier length_modifier;
    char conversion_specifier;
    int argument_index;
    bool is_custom_specifier;
};

struct pf_engine_token
{
    bool is_literal;
    ft_string literal_value;
    pf_engine_format_spec spec;
};

enum pf_engine_argument_kind
{
    PF_ARGUMENT_NONE,
    PF_ARGUMENT_INT,
    PF_ARGUMENT_UNSIGNED_INT,
    PF_ARGUMENT_LONG,
    PF_ARGUMENT_UNSIGNED_LONG,
    PF_ARGUMENT_LONG_LONG,
    PF_ARGUMENT_UNSIGNED_LONG_LONG,
    PF_ARGUMENT_INTMAX,
    PF_ARGUMENT_UINTMAX,
    PF_ARGUMENT_SIZE,
    PF_ARGUMENT_SIGNED_SIZE,
    PF_ARGUMENT_PTRDIFF,
    PF_ARGUMENT_UNSIGNED_PTRDIFF,
    PF_ARGUMENT_DOUBLE,
    PF_ARGUMENT_LONG_DOUBLE,
    PF_ARGUMENT_VOID_POINTER,
    PF_ARGUMENT_CSTRING,
    PF_ARGUMENT_WIDE_STRING,
    PF_ARGUMENT_WINT,
    PF_ARGUMENT_INT_POINTER,
    PF_ARGUMENT_LONG_POINTER,
    PF_ARGUMENT_LONG_LONG_POINTER,
    PF_ARGUMENT_SHORT_POINTER,
    PF_ARGUMENT_SIGNED_CHAR_POINTER,
    PF_ARGUMENT_INTMAX_POINTER,
    PF_ARGUMENT_SIZE_POINTER,
    PF_ARGUMENT_PTRDIFF_POINTER,
    PF_ARGUMENT_BOOLEAN
};

struct pf_engine_argument_value
{
    pf_engine_argument_kind kind;
    union
    {
        int int_value;
        unsigned int unsigned_int_value;
        long long_value;
        unsigned long unsigned_long_value;
        long long long_long_value;
        unsigned long long unsigned_long_long_value;
        intmax_t intmax_value;
        uintmax_t uintmax_value;
        size_t size_value;
        pf_signed_size_t signed_size_value;
        ptrdiff_t ptrdiff_value;
        pf_unsigned_ptrdiff_t unsigned_ptrdiff_value;
        double double_value;
        long double long_double_value;
        void *pointer_value;
        const char *cstring_value;
        const wchar_t *wide_string_value;
        wint_t wide_char_value;
        int *int_pointer_value;
        long *long_pointer_value;
        long long *long_long_pointer_value;
        short *short_pointer_value;
        signed char *signed_char_pointer_value;
        intmax_t *intmax_pointer_value;
        size_t *size_pointer_value;
        ptrdiff_t *ptrdiff_pointer_value;
    } data;
};

static bool pf_engine_is_digit(char character)
{
    if (character >= '0' && character <= '9')
        return (true);
    return (false);
}

static void pf_engine_append_unsigned_number(ft_string &target, unsigned int number)
{
    char buffer[32];
    int required_length;
    required_length = std::snprintf(buffer, sizeof(buffer), "%u", number);
    if (required_length <= 0)
        return ;
    size_t index;
    index = 0;
    while (index < static_cast<size_t>(required_length))
    {
        target.append(buffer + index, 1);
        index += 1;
    }
    return ;
}

static void pf_engine_append_signed_number(ft_string &target, int number)
{
    char buffer[32];
    int required_length;
    required_length = std::snprintf(buffer, sizeof(buffer), "%d", number);
    if (required_length <= 0)
        return ;
    size_t index;
    index = 0;
    while (index < static_cast<size_t>(required_length))
    {
        target.append(buffer + index, 1);
        index += 1;
    }
    return ;
}

static bool pf_engine_is_standard_conversion(char specifier)
{
    if (specifier == 'd' || specifier == 'i')
        return (true);
    if (specifier == 'u' || specifier == 'o')
        return (true);
    if (specifier == 'x' || specifier == 'X')
        return (true);
    if (specifier == 'f' || specifier == 'F')
        return (true);
    if (specifier == 'e' || specifier == 'E')
        return (true);
    if (specifier == 'g' || specifier == 'G')
        return (true);
    if (specifier == 'a' || specifier == 'A')
        return (true);
    if (specifier == 'c' || specifier == 's')
        return (true);
    if (specifier == 'p' || specifier == 'n')
        return (true);
    if (specifier == '%')
        return (true);
    if (specifier == 'b')
        return (true);
    return (false);
}

static void pf_engine_init_spec(pf_engine_format_spec &spec)
{
    spec.flag_left_align = false;
    spec.flag_force_sign = false;
    spec.flag_space_sign = false;
    spec.flag_alternate_form = false;
    spec.flag_zero_pad = false;
    spec.width_specified = false;
    spec.width_from_argument = false;
    spec.width_value = 0;
    spec.width_argument_index = -1;
    spec.precision_specified = false;
    spec.precision_from_argument = false;
    spec.precision_value = 0;
    spec.precision_argument_index = -1;
    spec.length_modifier = PF_LEN_NONE;
    spec.conversion_specifier = '\0';
    spec.argument_index = -1;
    spec.is_custom_specifier = false;
    return ;
}

static int pf_engine_parse_number(const char *format, size_t length, size_t *cursor, int *out_value)
{
    size_t index;
    int value;
    bool has_digit;

    index = *cursor;
    value = 0;
    has_digit = false;
    while (index < length)
    {
        char character;
        character = format[index];
        if (!pf_engine_is_digit(character))
            break ;
        has_digit = true;
        int digit_value;
        digit_value = character - '0';
        value = value * 10 + digit_value;
        index += 1;
    }
    if (!has_digit)
        return (0);
    *cursor = index;
    *out_value = value;
    return (1);
}

static int pf_engine_parse_specifier(const char *format, size_t length, size_t *cursor, pf_engine_format_spec &spec, bool *uses_positional, bool *uses_sequential)
{
    size_t index;

    pf_engine_init_spec(spec);
    index = *cursor;
    if (index >= length)
        return (-1);
    size_t positional_start;
    positional_start = index;
    int positional_value;
    positional_value = 0;
    size_t positional_cursor;
    positional_cursor = index;
    int positional_parse_status;
    positional_parse_status = pf_engine_parse_number(format, length, &positional_cursor, &positional_value);
    if (positional_parse_status == 1 && positional_cursor < length && format[positional_cursor] == '$')
    {
        spec.argument_index = positional_value - 1;
        *uses_positional = true;
        positional_cursor += 1;
        index = positional_cursor;
    }
    else
    {
        spec.argument_index = -1;
        index = positional_start;
        *uses_sequential = true;
    }
    bool parsing_flags;
    parsing_flags = true;
    while (parsing_flags && index < length)
    {
        char flag_character;
        flag_character = format[index];
        if (flag_character == '-')
            spec.flag_left_align = true;
        else if (flag_character == '+')
            spec.flag_force_sign = true;
        else if (flag_character == ' ')
            spec.flag_space_sign = true;
        else if (flag_character == '#')
            spec.flag_alternate_form = true;
        else if (flag_character == '0')
            spec.flag_zero_pad = true;
        else
        {
            parsing_flags = false;
            break ;
        }
        if (parsing_flags)
            index += 1;
    }
    if (index < length && format[index] == '*')
    {
        spec.width_specified = true;
        spec.width_from_argument = true;
        index += 1;
        size_t width_cursor;
        width_cursor = index;
        int width_position_value;
        width_position_value = 0;
        int width_position_status;
        width_position_status = pf_engine_parse_number(format, length, &width_cursor, &width_position_value);
        if (width_position_status == 1 && width_cursor < length && format[width_cursor] == '$')
        {
            spec.width_argument_index = width_position_value - 1;
            index = width_cursor + 1;
            *uses_positional = true;
        }
        else
        {
            spec.width_argument_index = -1;
            *uses_sequential = true;
        }
    }
    else
    {
        int width_value;
        size_t width_cursor;
        width_cursor = index;
        int width_status;
        width_status = pf_engine_parse_number(format, length, &width_cursor, &width_value);
        if (width_status == 1)
        {
            spec.width_specified = true;
            spec.width_from_argument = false;
            spec.width_value = width_value;
            index = width_cursor;
        }
    }
    if (index < length && format[index] == '.')
    {
        index += 1;
        spec.precision_specified = true;
        if (index < length && format[index] == '*')
        {
            spec.precision_from_argument = true;
            spec.precision_value = 0;
            index += 1;
            size_t precision_cursor;
            precision_cursor = index;
            int precision_position_value;
            precision_position_value = 0;
            int precision_position_status;
            precision_position_status = pf_engine_parse_number(format, length, &precision_cursor, &precision_position_value);
            if (precision_position_status == 1 && precision_cursor < length && format[precision_cursor] == '$')
            {
                spec.precision_argument_index = precision_position_value - 1;
                index = precision_cursor + 1;
                *uses_positional = true;
            }
            else
            {
                spec.precision_argument_index = -1;
                *uses_sequential = true;
            }
        }
        else
        {
            int precision_value;
            size_t precision_cursor;
            precision_cursor = index;
            int precision_status;
            precision_status = pf_engine_parse_number(format, length, &precision_cursor, &precision_value);
            if (precision_status == 1)
            {
                spec.precision_value = precision_value;
                index = precision_cursor;
            }
            else
                spec.precision_value = 0;
        }
    }
    if (index < length)
    {
        char modifier;
        modifier = format[index];
        if (modifier == 'h')
        {
            if (index + 1 < length && format[index + 1] == 'h')
            {
                spec.length_modifier = PF_LEN_HH;
                index += 2;
            }
            else
            {
                spec.length_modifier = PF_LEN_H;
                index += 1;
            }
        }
        else if (modifier == 'l')
        {
            if (index + 1 < length && format[index + 1] == 'l')
            {
                spec.length_modifier = PF_LEN_LL;
                index += 2;
            }
            else
            {
                spec.length_modifier = PF_LEN_L;
                index += 1;
            }
        }
        else if (modifier == 'j')
        {
            spec.length_modifier = PF_LEN_J;
            index += 1;
        }
        else if (modifier == 'z')
        {
            spec.length_modifier = PF_LEN_Z;
            index += 1;
        }
        else if (modifier == 't')
        {
            spec.length_modifier = PF_LEN_T;
            index += 1;
        }
        else if (modifier == 'L')
        {
            spec.length_modifier = PF_LEN_CAPITAL_L;
            index += 1;
        }
        else
            spec.length_modifier = PF_LEN_NONE;
    }
    if (index >= length)
        return (-1);
    spec.conversion_specifier = format[index];
    if (!pf_engine_is_standard_conversion(spec.conversion_specifier))
        spec.is_custom_specifier = true;
    *cursor = index + 1;
    return (0);
}

static int pf_engine_parse_format(const char *format, std::vector<pf_engine_token> &tokens, bool *uses_positional, bool *uses_sequential)
{
    size_t length;
    length = 0;
    while (format[length])
        length += 1;
    size_t index;
    index = 0;
    size_t literal_start;
    literal_start = 0;
    while (index < length)
    {
        char character;
        character = format[index];
        if (character != '%')
        {
            index += 1;
            continue ;
        }
        if (index > literal_start)
        {
            pf_engine_token literal_token;
            literal_token.is_literal = true;
            literal_token.literal_value.assign(format + literal_start, index - literal_start);
            tokens.push_back(literal_token);
        }
        index += 1;
        if (index >= length)
            return (-1);
        if (format[index] == '%')
        {
            pf_engine_token literal_token;
            literal_token.is_literal = true;
            literal_token.literal_value.assign("%", 1);
            tokens.push_back(literal_token);
            index += 1;
            literal_start = index;
            continue ;
        }
        pf_engine_format_spec spec;
        size_t spec_cursor;
        spec_cursor = index;
        if (pf_engine_parse_specifier(format, length, &spec_cursor, spec, uses_positional, uses_sequential) != 0)
            return (-1);
        pf_engine_token spec_token;
        spec_token.is_literal = false;
        spec_token.spec = spec;
        tokens.push_back(spec_token);
        index = spec_cursor;
        literal_start = index;
    }
    if (literal_start < length)
    {
        pf_engine_token literal_token;
        literal_token.is_literal = true;
        literal_token.literal_value.assign(format + literal_start, length - literal_start);
        tokens.push_back(literal_token);
    }
    return (0);
}

static int pf_engine_write_literal(const ft_string &literal, t_pf_engine_write_callback writer, void *context, size_t *written_count)
{
    int write_error;

    if (literal.size() == 0)
        return (FT_ERR_SUCCESSS);
    write_error = writer(literal.c_str(), literal.size(), context, written_count);
    if (write_error != FT_ERR_SUCCESSS)
        return (write_error);
    return (FT_ERR_SUCCESSS);
}

static ft_string pf_engine_build_format_string(const pf_engine_format_spec &spec, int width_value, bool width_specified, int precision_value, bool precision_specified)
{
    ft_string format_string;
    format_string.append("%", 1);
    if (spec.argument_index >= 0)
    {
        pf_engine_append_signed_number(format_string, spec.argument_index + 1);
        format_string.append("$", 1);
    }
    if (spec.flag_left_align)
        format_string.append("-", 1);
    if (spec.flag_force_sign)
        format_string.append("+", 1);
    if (spec.flag_space_sign)
        format_string.append(" ", 1);
    if (spec.flag_alternate_form)
        format_string.append("#", 1);
    if (spec.flag_zero_pad && !spec.flag_left_align)
        format_string.append("0", 1);
    if (width_specified && width_value >= 0)
        pf_engine_append_unsigned_number(format_string, static_cast<unsigned int>(width_value));
    if (precision_specified)
    {
        format_string.append(".", 1);
        if (precision_value >= 0)
            pf_engine_append_unsigned_number(format_string, static_cast<unsigned int>(precision_value));
    }
    if (spec.length_modifier == PF_LEN_HH)
        format_string.append("hh", 2);
    else if (spec.length_modifier == PF_LEN_H)
        format_string.append("h", 1);
    else if (spec.length_modifier == PF_LEN_L)
        format_string.append("l", 1);
    else if (spec.length_modifier == PF_LEN_LL)
        format_string.append("ll", 2);
    else if (spec.length_modifier == PF_LEN_J)
        format_string.append("j", 1);
    else if (spec.length_modifier == PF_LEN_Z)
        format_string.append("z", 1);
    else if (spec.length_modifier == PF_LEN_T)
        format_string.append("t", 1);
    else if (spec.length_modifier == PF_LEN_CAPITAL_L)
        format_string.append("L", 1);
    char specifier_buffer[2];
    specifier_buffer[0] = spec.conversion_specifier;
    specifier_buffer[1] = '\0';
    format_string.append(specifier_buffer, 1);
    return (format_string);
}

static int pf_engine_format_boolean(int value, t_pf_engine_write_callback writer, void *context, size_t *written_count)
{
    const char *literal;
    size_t length;
    int write_error;

    if (value)
    {
        literal = "true";
        length = 4;
    }
    else
    {
        literal = "false";
        length = 5;
    }
    write_error = writer(literal, length, context, written_count);
    if (write_error != FT_ERR_SUCCESSS)
        return (write_error);
    return (FT_ERR_SUCCESSS);
}

static int pf_engine_format_with_snprintf(const ft_string &format_string, t_pf_engine_write_callback writer, void *context, size_t *written_count, ...)
{
    va_list args;
    int write_error;

    va_start(args, written_count);
    int required_length;
    required_length = std::vsnprintf(ft_nullptr, 0, format_string.c_str(), args);
    va_end(args);
    if (required_length < 0)
        return (FT_ERR_IO);
    if (required_length == 0)
    {
        write_error = writer("", 0, context, written_count);
        if (write_error != FT_ERR_SUCCESSS)
            return (write_error);
        return (FT_ERR_SUCCESSS);
    }
    ft_string output;
    output.resize_length(static_cast<size_t>(required_length));
    if (output.get_error() != FT_ERR_SUCCESSS)
        return (output.get_error());
    va_start(args, written_count);
    int written_length;
    written_length = std::vsnprintf(output.print(), static_cast<size_t>(required_length) + 1, format_string.c_str(), args);
    va_end(args);
    if (written_length < 0)
        return (FT_ERR_IO);
    output.resize_length(static_cast<size_t>(written_length));
    write_error = writer(output.c_str(), output.size(), context, written_count);
    if (write_error != FT_ERR_SUCCESSS)
        return (write_error);
    return (FT_ERR_SUCCESSS);
}

static int pf_engine_format_standard_sequential(const pf_engine_format_spec &original_spec, va_list *args, t_pf_engine_write_callback writer, void *context, size_t *written_count, size_t *character_count)
{
    pf_engine_format_spec spec;
    spec = original_spec;
    int width_value;
    bool width_specified;
    width_specified = spec.width_specified;
    width_value = spec.width_value;
    if (spec.width_from_argument)
    {
        int dynamic_width;
        dynamic_width = va_arg(*args, int);
        if (dynamic_width < 0)
        {
            spec.flag_left_align = true;
            width_value = -dynamic_width;
            width_specified = true;
        }
        else
        {
            width_value = dynamic_width;
            width_specified = true;
        }
    }
    int precision_value;
    bool precision_specified;
    precision_specified = spec.precision_specified;
    precision_value = spec.precision_value;
    if (spec.precision_from_argument)
    {
        int dynamic_precision;
        dynamic_precision = va_arg(*args, int);
        if (dynamic_precision >= 0)
        {
            precision_value = dynamic_precision;
            precision_specified = true;
        }
        else
            precision_specified = false;
    }
    if (spec.conversion_specifier == 'n')
    {
        size_t value;
        value = *character_count;
        if (spec.length_modifier == PF_LEN_HH)
        {
            signed char *pointer;
            pointer = va_arg(*args, signed char*);
            if (pointer)
                *pointer = static_cast<signed char>(value);
        }
        else if (spec.length_modifier == PF_LEN_H)
        {
            short *pointer;
            pointer = va_arg(*args, short*);
            if (pointer)
                *pointer = static_cast<short>(value);
        }
        else if (spec.length_modifier == PF_LEN_L)
        {
            long *pointer;
            pointer = va_arg(*args, long*);
            if (pointer)
                *pointer = static_cast<long>(value);
        }
        else if (spec.length_modifier == PF_LEN_LL)
        {
            long long *pointer;
            pointer = va_arg(*args, long long*);
            if (pointer)
                *pointer = static_cast<long long>(value);
        }
        else if (spec.length_modifier == PF_LEN_J)
        {
            intmax_t *pointer;
            pointer = va_arg(*args, intmax_t*);
            if (pointer)
                *pointer = static_cast<intmax_t>(value);
        }
        else if (spec.length_modifier == PF_LEN_Z)
        {
            size_t *pointer;
            pointer = va_arg(*args, size_t*);
            if (pointer)
                *pointer = value;
        }
        else if (spec.length_modifier == PF_LEN_T)
        {
            ptrdiff_t *pointer;
            pointer = va_arg(*args, ptrdiff_t*);
            if (pointer)
                *pointer = static_cast<ptrdiff_t>(value);
        }
        else
        {
            int *pointer;
            pointer = va_arg(*args, int*);
            if (pointer)
                *pointer = static_cast<int>(value);
        }
        return (FT_ERR_SUCCESSS);
    }
    if (spec.conversion_specifier == 'b')
    {
        int boolean_value;
        int format_error;

        boolean_value = va_arg(*args, int);
        format_error = pf_engine_format_boolean(boolean_value, writer, context, written_count);
        if (format_error != FT_ERR_SUCCESSS)
            return (format_error);
        *character_count = *written_count;
        return (FT_ERR_SUCCESSS);
    }
    ft_string format_string;
    format_string = pf_engine_build_format_string(spec, width_value, width_specified, precision_value, precision_specified);
    int status;
    if (spec.conversion_specifier == 'd' || spec.conversion_specifier == 'i')
    {
        if (spec.length_modifier == PF_LEN_HH)
        {
            int value;
            value = va_arg(*args, int);
            signed char converted;
            converted = static_cast<signed char>(value);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, converted);
        }
        else if (spec.length_modifier == PF_LEN_H)
        {
            int value;
            value = va_arg(*args, int);
            short converted;
            converted = static_cast<short>(value);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, converted);
        }
        else if (spec.length_modifier == PF_LEN_L)
        {
            long value;
            value = va_arg(*args, long);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (spec.length_modifier == PF_LEN_LL)
        {
            long long value;
            value = va_arg(*args, long long);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (spec.length_modifier == PF_LEN_J)
        {
            intmax_t value;
            value = va_arg(*args, intmax_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (spec.length_modifier == PF_LEN_Z)
        {
            pf_signed_size_t value;
            value = va_arg(*args, pf_signed_size_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (spec.length_modifier == PF_LEN_T)
        {
            ptrdiff_t value;
            value = va_arg(*args, ptrdiff_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else
        {
            int value;
            value = va_arg(*args, int);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
    }
    else if (spec.conversion_specifier == 'u' || spec.conversion_specifier == 'o' || spec.conversion_specifier == 'x' || spec.conversion_specifier == 'X')
    {
        if (spec.length_modifier == PF_LEN_HH)
        {
            unsigned int value;
            value = va_arg(*args, unsigned int);
            unsigned char converted;
            converted = static_cast<unsigned char>(value);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, converted);
        }
        else if (spec.length_modifier == PF_LEN_H)
        {
            unsigned int value;
            value = va_arg(*args, unsigned int);
            unsigned short converted;
            converted = static_cast<unsigned short>(value);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, converted);
        }
        else if (spec.length_modifier == PF_LEN_L)
        {
            unsigned long value;
            value = va_arg(*args, unsigned long);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (spec.length_modifier == PF_LEN_LL)
        {
            unsigned long long value;
            value = va_arg(*args, unsigned long long);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (spec.length_modifier == PF_LEN_J)
        {
            uintmax_t value;
            value = va_arg(*args, uintmax_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (spec.length_modifier == PF_LEN_Z)
        {
            size_t value;
            value = va_arg(*args, size_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (spec.length_modifier == PF_LEN_T)
        {
            pf_unsigned_ptrdiff_t value;
            value = va_arg(*args, pf_unsigned_ptrdiff_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else
        {
            unsigned int value;
            value = va_arg(*args, unsigned int);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
    }
    else if (spec.conversion_specifier == 'f' || spec.conversion_specifier == 'F' || spec.conversion_specifier == 'e' || spec.conversion_specifier == 'E' || spec.conversion_specifier == 'g' || spec.conversion_specifier == 'G' || spec.conversion_specifier == 'a' || spec.conversion_specifier == 'A')
    {
        if (spec.length_modifier == PF_LEN_CAPITAL_L)
        {
            long double value;
            value = va_arg(*args, long double);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else
        {
            double value;
            value = va_arg(*args, double);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
    }
    else if (spec.conversion_specifier == 'c')
    {
        if (spec.length_modifier == PF_LEN_L)
        {
            wint_t value;
            value = va_arg(*args, wint_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else
        {
            int value;
            value = va_arg(*args, int);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
    }
    else if (spec.conversion_specifier == 's')
    {
        if (spec.length_modifier == PF_LEN_L)
        {
            const wchar_t *value;
            value = va_arg(*args, const wchar_t*);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else
        {
            const char *value;
            value = va_arg(*args, const char*);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
    }
    else if (spec.conversion_specifier == 'p')
    {
        void *value;
        value = va_arg(*args, void*);
        status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
    }
    else if (spec.conversion_specifier == '%')
    {
        int write_error;

        write_error = writer("%", 1, context, written_count);
        if (write_error != FT_ERR_SUCCESSS)
            return (write_error);
        *character_count += 1;
        return (FT_ERR_SUCCESSS);
    }
    else
        return (FT_ERR_INVALID_ARGUMENT);
    if (status != FT_ERR_SUCCESSS)
        return (status);
    *character_count = *written_count;
    return (FT_ERR_SUCCESSS);
}

static int pf_engine_process_sequential(const std::vector<pf_engine_token> &tokens, va_list args, t_pf_engine_write_callback writer, void *context, size_t *written_count)
{
    size_t index;
    index = 0;
    size_t character_count;
    character_count = *written_count;
    va_list current_args;
    va_copy(current_args, args);
    int status;
    status = FT_ERR_SUCCESSS;
    bool mark_overflow;
    mark_overflow = false;
    while (index < tokens.size())
    {
        const pf_engine_token &token = tokens[index];
        if (token.is_literal)
        {
            status = pf_engine_write_literal(token.literal_value, writer, context, written_count);
            if (status != FT_ERR_SUCCESSS)
                break ;
            character_count = *written_count;
        }
        else
        {
            if (token.spec.is_custom_specifier)
            {
                ft_string custom_output;
                bool handled;

                handled = false;
                status = pf_try_format_custom_specifier(token.spec.conversion_specifier, &current_args, custom_output, &handled);
                if (status != FT_ERR_SUCCESSS)
                {
                    mark_overflow = true;
                    break ;
                }
                if (handled)
                {
                    status = pf_engine_write_literal(custom_output, writer, context, written_count);
                    if (status != FT_ERR_SUCCESSS)
                        break ;
                    character_count = *written_count;
                }
                else
                {
                    status = writer("%", 1, context, written_count);
                    if (status != FT_ERR_SUCCESSS)
                        break ;
                    status = writer(&token.spec.conversion_specifier, 1, context, written_count);
                    if (status != FT_ERR_SUCCESSS)
                        break ;
                    character_count = *written_count;
                }
                index += 1;
                continue ;
            }
            status = pf_engine_format_standard_sequential(token.spec, &current_args, writer, context, written_count, &character_count);
            if (status != FT_ERR_SUCCESSS)
                break ;
        }
        index += 1;
    }
    va_end(current_args);
    if (status != FT_ERR_SUCCESSS)
    {
        if (mark_overflow == true)
            *written_count = SIZE_MAX;
        return (status);
    }
    *written_count = character_count;
    return (FT_ERR_SUCCESSS);
}

static void pf_engine_argument_types_resize(std::vector<pf_engine_argument_kind> &kinds, size_t required_index)
{
    while (kinds.size() <= required_index)
    {
        kinds.push_back(PF_ARGUMENT_NONE);
    }
}

static int pf_engine_register_argument_kind(std::vector<pf_engine_argument_kind> &kinds, size_t index, pf_engine_argument_kind kind)
{
    pf_engine_argument_types_resize(kinds, index);
    if (kinds[index] == PF_ARGUMENT_NONE)
    {
        kinds[index] = kind;
        return (0);
    }
    if (kinds[index] == kind)
        return (0);
    return (-1);
}

static int pf_engine_collect_argument_kinds(const std::vector<pf_engine_token> &tokens, std::vector<pf_engine_argument_kind> &kinds)
{
    size_t index;
    index = 0;
    while (index < tokens.size())
    {
        const pf_engine_token &token = tokens[index];
        if (token.is_literal)
        {
            index += 1;
            continue ;
        }
        const pf_engine_format_spec &spec = token.spec;
        if (spec.width_from_argument && spec.width_argument_index >= 0)
        {
            if (pf_engine_register_argument_kind(kinds, static_cast<size_t>(spec.width_argument_index), PF_ARGUMENT_INT) != 0)
                return (-1);
        }
        if (spec.precision_from_argument && spec.precision_argument_index >= 0)
        {
            if (pf_engine_register_argument_kind(kinds, static_cast<size_t>(spec.precision_argument_index), PF_ARGUMENT_INT) != 0)
                return (-1);
        }
        if (spec.conversion_specifier == '%' || spec.conversion_specifier == '\0')
        {
            index += 1;
            continue ;
        }
        if (spec.conversion_specifier == 'n')
        {
            pf_engine_argument_kind kind;
            if (spec.length_modifier == PF_LEN_HH)
                kind = PF_ARGUMENT_SIGNED_CHAR_POINTER;
            else if (spec.length_modifier == PF_LEN_H)
                kind = PF_ARGUMENT_SHORT_POINTER;
            else if (spec.length_modifier == PF_LEN_L)
                kind = PF_ARGUMENT_LONG_POINTER;
            else if (spec.length_modifier == PF_LEN_LL)
                kind = PF_ARGUMENT_LONG_LONG_POINTER;
            else if (spec.length_modifier == PF_LEN_J)
                kind = PF_ARGUMENT_INTMAX_POINTER;
            else if (spec.length_modifier == PF_LEN_Z)
                kind = PF_ARGUMENT_SIZE_POINTER;
            else if (spec.length_modifier == PF_LEN_T)
                kind = PF_ARGUMENT_PTRDIFF_POINTER;
            else
                kind = PF_ARGUMENT_INT_POINTER;
            if (spec.argument_index < 0)
                return (-1);
            if (pf_engine_register_argument_kind(kinds, static_cast<size_t>(spec.argument_index), kind) != 0)
                return (-1);
            index += 1;
            continue ;
        }
        if (spec.conversion_specifier == 'b')
        {
            if (spec.argument_index < 0)
                return (-1);
            if (pf_engine_register_argument_kind(kinds, static_cast<size_t>(spec.argument_index), PF_ARGUMENT_BOOLEAN) != 0)
                return (-1);
            index += 1;
            continue ;
        }
        pf_engine_argument_kind kind;
        kind = PF_ARGUMENT_NONE;
        if (spec.conversion_specifier == 'd' || spec.conversion_specifier == 'i')
        {
            if (spec.length_modifier == PF_LEN_HH || spec.length_modifier == PF_LEN_H)
                kind = PF_ARGUMENT_INT;
            else if (spec.length_modifier == PF_LEN_L)
                kind = PF_ARGUMENT_LONG;
            else if (spec.length_modifier == PF_LEN_LL)
                kind = PF_ARGUMENT_LONG_LONG;
            else if (spec.length_modifier == PF_LEN_J)
                kind = PF_ARGUMENT_INTMAX;
            else if (spec.length_modifier == PF_LEN_Z)
                kind = PF_ARGUMENT_SIGNED_SIZE;
            else if (spec.length_modifier == PF_LEN_T)
                kind = PF_ARGUMENT_PTRDIFF;
            else
                kind = PF_ARGUMENT_INT;
        }
        else if (spec.conversion_specifier == 'u' || spec.conversion_specifier == 'o' || spec.conversion_specifier == 'x' || spec.conversion_specifier == 'X')
        {
            if (spec.length_modifier == PF_LEN_HH || spec.length_modifier == PF_LEN_H)
                kind = PF_ARGUMENT_UNSIGNED_INT;
            else if (spec.length_modifier == PF_LEN_L)
                kind = PF_ARGUMENT_UNSIGNED_LONG;
            else if (spec.length_modifier == PF_LEN_LL)
                kind = PF_ARGUMENT_UNSIGNED_LONG_LONG;
            else if (spec.length_modifier == PF_LEN_J)
                kind = PF_ARGUMENT_UINTMAX;
            else if (spec.length_modifier == PF_LEN_Z)
                kind = PF_ARGUMENT_SIZE;
            else if (spec.length_modifier == PF_LEN_T)
                kind = PF_ARGUMENT_UNSIGNED_PTRDIFF;
            else
                kind = PF_ARGUMENT_UNSIGNED_INT;
        }
        else if (spec.conversion_specifier == 'f' || spec.conversion_specifier == 'F' || spec.conversion_specifier == 'e' || spec.conversion_specifier == 'E' || spec.conversion_specifier == 'g' || spec.conversion_specifier == 'G' || spec.conversion_specifier == 'a' || spec.conversion_specifier == 'A')
        {
            if (spec.length_modifier == PF_LEN_CAPITAL_L)
                kind = PF_ARGUMENT_LONG_DOUBLE;
            else
                kind = PF_ARGUMENT_DOUBLE;
        }
        else if (spec.conversion_specifier == 'c')
        {
            if (spec.length_modifier == PF_LEN_L)
                kind = PF_ARGUMENT_WINT;
            else
                kind = PF_ARGUMENT_INT;
        }
        else if (spec.conversion_specifier == 's')
        {
            if (spec.length_modifier == PF_LEN_L)
                kind = PF_ARGUMENT_WIDE_STRING;
            else
                kind = PF_ARGUMENT_CSTRING;
        }
        else if (spec.conversion_specifier == 'p')
            kind = PF_ARGUMENT_VOID_POINTER;
        else
            return (-1);
        if (spec.argument_index < 0)
            return (-1);
        if (pf_engine_register_argument_kind(kinds, static_cast<size_t>(spec.argument_index), kind) != 0)
            return (-1);
        index += 1;
    }
    return (0);
}

static int pf_engine_store_arguments(const std::vector<pf_engine_argument_kind> &kinds, va_list args, std::vector<pf_engine_argument_value> &values)
{
    size_t index;
    index = 0;
    va_list current_args;
    va_copy(current_args, args);
    while (index < kinds.size())
    {
        pf_engine_argument_kind kind;
        kind = kinds[index];
        pf_engine_argument_value value;
        value.kind = kind;
        if (kind == PF_ARGUMENT_INT)
            value.data.int_value = va_arg(current_args, int);
        else if (kind == PF_ARGUMENT_UNSIGNED_INT)
            value.data.unsigned_int_value = va_arg(current_args, unsigned int);
        else if (kind == PF_ARGUMENT_LONG)
            value.data.long_value = va_arg(current_args, long);
        else if (kind == PF_ARGUMENT_UNSIGNED_LONG)
            value.data.unsigned_long_value = va_arg(current_args, unsigned long);
        else if (kind == PF_ARGUMENT_LONG_LONG)
            value.data.long_long_value = va_arg(current_args, long long);
        else if (kind == PF_ARGUMENT_UNSIGNED_LONG_LONG)
            value.data.unsigned_long_long_value = va_arg(current_args, unsigned long long);
        else if (kind == PF_ARGUMENT_INTMAX)
            value.data.intmax_value = va_arg(current_args, intmax_t);
        else if (kind == PF_ARGUMENT_UINTMAX)
            value.data.uintmax_value = va_arg(current_args, uintmax_t);
        else if (kind == PF_ARGUMENT_SIZE)
            value.data.size_value = va_arg(current_args, size_t);
        else if (kind == PF_ARGUMENT_SIGNED_SIZE)
            value.data.signed_size_value = va_arg(current_args, pf_signed_size_t);
        else if (kind == PF_ARGUMENT_PTRDIFF)
            value.data.ptrdiff_value = va_arg(current_args, ptrdiff_t);
        else if (kind == PF_ARGUMENT_UNSIGNED_PTRDIFF)
            value.data.unsigned_ptrdiff_value = va_arg(current_args, pf_unsigned_ptrdiff_t);
        else if (kind == PF_ARGUMENT_DOUBLE)
            value.data.double_value = va_arg(current_args, double);
        else if (kind == PF_ARGUMENT_LONG_DOUBLE)
            value.data.long_double_value = va_arg(current_args, long double);
        else if (kind == PF_ARGUMENT_VOID_POINTER)
            value.data.pointer_value = va_arg(current_args, void*);
        else if (kind == PF_ARGUMENT_CSTRING)
            value.data.cstring_value = va_arg(current_args, const char*);
        else if (kind == PF_ARGUMENT_WIDE_STRING)
            value.data.wide_string_value = va_arg(current_args, const wchar_t*);
        else if (kind == PF_ARGUMENT_WINT)
            value.data.wide_char_value = va_arg(current_args, wint_t);
        else if (kind == PF_ARGUMENT_INT_POINTER)
            value.data.int_pointer_value = va_arg(current_args, int*);
        else if (kind == PF_ARGUMENT_LONG_POINTER)
            value.data.long_pointer_value = va_arg(current_args, long*);
        else if (kind == PF_ARGUMENT_LONG_LONG_POINTER)
            value.data.long_long_pointer_value = va_arg(current_args, long long*);
        else if (kind == PF_ARGUMENT_SHORT_POINTER)
            value.data.short_pointer_value = va_arg(current_args, short*);
        else if (kind == PF_ARGUMENT_SIGNED_CHAR_POINTER)
            value.data.signed_char_pointer_value = va_arg(current_args, signed char*);
        else if (kind == PF_ARGUMENT_INTMAX_POINTER)
            value.data.intmax_pointer_value = va_arg(current_args, intmax_t*);
        else if (kind == PF_ARGUMENT_SIZE_POINTER)
            value.data.size_pointer_value = va_arg(current_args, size_t*);
        else if (kind == PF_ARGUMENT_PTRDIFF_POINTER)
            value.data.ptrdiff_pointer_value = va_arg(current_args, ptrdiff_t*);
        else if (kind == PF_ARGUMENT_BOOLEAN)
            value.data.int_value = va_arg(current_args, int);
        values.push_back(value);
        index += 1;
    }
    va_end(current_args);
    return (0);
}

static int pf_engine_fetch_int_argument(const std::vector<pf_engine_argument_value> &values, int index, int *out_value)
{
    if (index < 0)
        return (-1);
    if (static_cast<size_t>(index) >= values.size())
        return (-1);
    const pf_engine_argument_value &value = values[static_cast<size_t>(index)];
    if (value.kind != PF_ARGUMENT_INT && value.kind != PF_ARGUMENT_BOOLEAN)
        return (-1);
    *out_value = value.data.int_value;
    return (0);
}

static int pf_engine_format_standard_positional(const pf_engine_format_spec &original_spec, const std::vector<pf_engine_argument_value> &values, t_pf_engine_write_callback writer, void *context, size_t *written_count, size_t *character_count)
{
    pf_engine_format_spec spec;
    spec = original_spec;
    int width_value;
    bool width_specified;
    width_specified = spec.width_specified;
    width_value = spec.width_value;
    if (spec.width_from_argument && spec.width_argument_index >= 0)
    {
        if (pf_engine_fetch_int_argument(values, spec.width_argument_index, &width_value) != 0)
            return (FT_ERR_INVALID_ARGUMENT);
        if (width_value < 0)
        {
            spec.flag_left_align = true;
            width_value = -width_value;
        }
        width_specified = true;
    }
    int precision_value;
    bool precision_specified;
    precision_specified = spec.precision_specified;
    precision_value = spec.precision_value;
    if (spec.precision_from_argument && spec.precision_argument_index >= 0)
    {
        if (pf_engine_fetch_int_argument(values, spec.precision_argument_index, &precision_value) != 0)
            return (FT_ERR_INVALID_ARGUMENT);
        if (precision_value >= 0)
            precision_specified = true;
        else
            precision_specified = false;
    }
    if (spec.conversion_specifier == 'n')
    {
        size_t value;
        value = *character_count;
        if (spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        pf_engine_argument_value target;
        target = values[static_cast<size_t>(spec.argument_index)];
        if (target.kind == PF_ARGUMENT_SIGNED_CHAR_POINTER && target.data.signed_char_pointer_value)
            *target.data.signed_char_pointer_value = static_cast<signed char>(value);
        else if (target.kind == PF_ARGUMENT_SHORT_POINTER && target.data.short_pointer_value)
            *target.data.short_pointer_value = static_cast<short>(value);
        else if (target.kind == PF_ARGUMENT_LONG_POINTER && target.data.long_pointer_value)
            *target.data.long_pointer_value = static_cast<long>(value);
        else if (target.kind == PF_ARGUMENT_LONG_LONG_POINTER && target.data.long_long_pointer_value)
            *target.data.long_long_pointer_value = static_cast<long long>(value);
        else if (target.kind == PF_ARGUMENT_INTMAX_POINTER && target.data.intmax_pointer_value)
            *target.data.intmax_pointer_value = static_cast<intmax_t>(value);
        else if (target.kind == PF_ARGUMENT_SIZE_POINTER && target.data.size_pointer_value)
            *target.data.size_pointer_value = value;
        else if (target.kind == PF_ARGUMENT_PTRDIFF_POINTER && target.data.ptrdiff_pointer_value)
            *target.data.ptrdiff_pointer_value = static_cast<ptrdiff_t>(value);
        else if (target.kind == PF_ARGUMENT_INT_POINTER && target.data.int_pointer_value)
            *target.data.int_pointer_value = static_cast<int>(value);
        return (FT_ERR_SUCCESSS);
    }
    if (spec.conversion_specifier == 'b')
    {
        if (spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<size_t>(spec.argument_index)];
        if (value.kind != PF_ARGUMENT_BOOLEAN)
            return (FT_ERR_INVALID_ARGUMENT);
        int format_error;

        format_error = pf_engine_format_boolean(value.data.int_value, writer, context, written_count);
        if (format_error != FT_ERR_SUCCESSS)
            return (format_error);
        *character_count = *written_count;
        return (FT_ERR_SUCCESSS);
    }
    ft_string format_string;
    format_string = pf_engine_build_format_string(spec, width_value, width_specified, precision_value, precision_specified);
    int status;
    status = FT_ERR_INVALID_ARGUMENT;
    if (spec.conversion_specifier == 'd' || spec.conversion_specifier == 'i')
    {
        if (spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<size_t>(spec.argument_index)];
        if (spec.length_modifier == PF_LEN_HH)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, static_cast<signed char>(value.data.int_value));
        else if (spec.length_modifier == PF_LEN_H)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, static_cast<short>(value.data.int_value));
        else if (spec.length_modifier == PF_LEN_L)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.long_value);
        else if (spec.length_modifier == PF_LEN_LL)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.long_long_value);
        else if (spec.length_modifier == PF_LEN_J)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.intmax_value);
        else if (spec.length_modifier == PF_LEN_Z)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.signed_size_value);
        else if (spec.length_modifier == PF_LEN_T)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.ptrdiff_value);
        else
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.int_value);
    }
    else if (spec.conversion_specifier == 'u' || spec.conversion_specifier == 'o' || spec.conversion_specifier == 'x' || spec.conversion_specifier == 'X')
    {
        if (spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<size_t>(spec.argument_index)];
        if (spec.length_modifier == PF_LEN_HH)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, static_cast<unsigned char>(value.data.unsigned_int_value));
        else if (spec.length_modifier == PF_LEN_H)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, static_cast<unsigned short>(value.data.unsigned_int_value));
        else if (spec.length_modifier == PF_LEN_L)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.unsigned_long_value);
        else if (spec.length_modifier == PF_LEN_LL)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.unsigned_long_long_value);
        else if (spec.length_modifier == PF_LEN_J)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.uintmax_value);
        else if (spec.length_modifier == PF_LEN_Z)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.size_value);
        else if (spec.length_modifier == PF_LEN_T)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.unsigned_ptrdiff_value);
        else
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.unsigned_int_value);
    }
    else if (spec.conversion_specifier == 'f' || spec.conversion_specifier == 'F' || spec.conversion_specifier == 'e' || spec.conversion_specifier == 'E' || spec.conversion_specifier == 'g' || spec.conversion_specifier == 'G' || spec.conversion_specifier == 'a' || spec.conversion_specifier == 'A')
    {
        if (spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<size_t>(spec.argument_index)];
        if (spec.length_modifier == PF_LEN_CAPITAL_L)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.long_double_value);
        else
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.double_value);
    }
    else if (spec.conversion_specifier == 'c')
    {
        if (spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<size_t>(spec.argument_index)];
        if (spec.length_modifier == PF_LEN_L)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.wide_char_value);
        else
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.int_value);
    }
    else if (spec.conversion_specifier == 's')
    {
        if (spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<size_t>(spec.argument_index)];
        if (spec.length_modifier == PF_LEN_L)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.wide_string_value);
        else
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.cstring_value);
    }
    else if (spec.conversion_specifier == 'p')
    {
        if (spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<size_t>(spec.argument_index)];
        status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.pointer_value);
    }
    else if (spec.conversion_specifier == '%')
    {
        int write_error;

        write_error = writer("%", 1, context, written_count);
        if (write_error != FT_ERR_SUCCESSS)
            return (write_error);
        *character_count += 1;
        return (FT_ERR_SUCCESSS);
    }
    if (status != FT_ERR_SUCCESSS)
        return (status);
    *character_count = *written_count;
    return (FT_ERR_SUCCESSS);
}

static int pf_engine_process_positional(const std::vector<pf_engine_token> &tokens, va_list args, t_pf_engine_write_callback writer, void *context, size_t *written_count)
{
    std::vector<pf_engine_argument_kind> kinds;
    if (pf_engine_collect_argument_kinds(tokens, kinds) != 0)
        return (FT_ERR_INVALID_ARGUMENT);
    std::vector<pf_engine_argument_value> values;
    if (pf_engine_store_arguments(kinds, args, values) != 0)
        return (FT_ERR_INVALID_ARGUMENT);
    size_t index;
    index = 0;
    size_t character_count;
    character_count = *written_count;
    while (index < tokens.size())
    {
        const pf_engine_token &token = tokens[index];
        if (token.is_literal)
        {
            int write_error;

            write_error = pf_engine_write_literal(token.literal_value, writer, context, written_count);
            if (write_error != FT_ERR_SUCCESSS)
                return (write_error);
            character_count = *written_count;
        }
        else
        {
            if (token.spec.is_custom_specifier)
                return (FT_ERR_INVALID_ARGUMENT);
            int format_error;

            format_error = pf_engine_format_standard_positional(token.spec, values, writer, context, written_count, &character_count);
            if (format_error != FT_ERR_SUCCESSS)
                return (format_error);
        }
        index += 1;
    }
    *written_count = character_count;
    return (FT_ERR_SUCCESSS);
}

int pf_engine_format(const char *format, va_list args, t_pf_engine_write_callback writer, void *context, size_t *written_count)
{
    if (!format || !written_count || !writer)
        return (FT_ERR_INVALID_ARGUMENT);
    std::vector<pf_engine_token> tokens;
    bool uses_positional;
    bool uses_sequential;
    uses_positional = false;
    uses_sequential = false;
    if (pf_engine_parse_format(format, tokens, &uses_positional, &uses_sequential) != 0)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (uses_positional && uses_sequential)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    size_t initial_count;
    initial_count = *written_count;
    int status;
    if (uses_positional)
        status = pf_engine_process_positional(tokens, args, writer, context, written_count);
    else
        status = pf_engine_process_sequential(tokens, args, writer, context, written_count);
    if (status != FT_ERR_SUCCESSS)
        return (status);
    if (*written_count < initial_count)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    return (FT_ERR_SUCCESSS);
}
