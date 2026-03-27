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
#include <new>
#include <type_traits>

typedef typename std::make_signed<ft_size_t>::type pf_signed_size_t;
typedef uint64_t pf_unsigned_ptrdiff_t;

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
    ft_bool flag_left_align;
    ft_bool flag_force_sign;
    ft_bool flag_space_sign;
    ft_bool flag_alternate_form;
    ft_bool flag_zero_pad;
    ft_bool width_specified;
    ft_bool width_from_argument;
    int32_t width_value;
    int32_t width_argument_index;
    ft_bool precision_specified;
    ft_bool precision_from_argument;
    int32_t precision_value;
    int32_t precision_argument_index;
    pf_length_modifier length_modifier;
    char conversion_specifier;
    int32_t argument_index;
    ft_bool is_custom_specifier;
};

struct pf_engine_token
{
    ft_bool is_literal;
    ft_string literal_value;
    ft_bool literal_initialised;
    pf_engine_format_spec format_spec;

    pf_engine_token() noexcept;
    ~pf_engine_token();
    int32_t ensure_literal_initialised() noexcept;
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
        int32_t int_value;
        uint32_t unsigned_int_value;
        int64_t long_value;
        uint64_t unsigned_long_value;
        int64_t long_long_value;
        uint64_t unsigned_long_long_value;
        int64_t intmax_value;
        uint64_t uintmax_value;
        ft_size_t size_value;
        pf_signed_size_t signed_size_value;
        int64_t ptrdiff_value;
        pf_unsigned_ptrdiff_t unsigned_ptrdiff_value;
        double double_value;
        long double long_double_value;
        void *pointer_value;
        const char *cstring_value;
        const wchar_t *wide_string_value;
        int32_t wide_char_value;
        int32_t *int_pointer_value;
        int64_t *long_pointer_value;
        int64_t *long_long_pointer_value;
        int16_t *short_pointer_value;
        int8_t *signed_char_pointer_value;
        int64_t *intmax_pointer_value;
        ft_size_t *size_pointer_value;
        int64_t *ptrdiff_pointer_value;
    } data;
};

static ft_bool pf_engine_is_digit(char character)
{
    if (character >= '0' && character <= '9')
        return (FT_TRUE);
    return (FT_FALSE);
}

static void pf_engine_append_unsigned_number(ft_string &target, uint32_t number)
{
    char buffer[32];
    int32_t required_length;
    required_length = std::snprintf(buffer, sizeof(buffer), "%u", number);
    if (required_length <= 0)
        return ;
    ft_size_t index;
    index = 0;
    while (index < static_cast<ft_size_t>(required_length))
    {
        target.append(buffer + index, 1);
        index += 1;
    }
    return ;
}

static void pf_engine_append_signed_number(ft_string &target, int32_t number)
{
    char buffer[32];
    int32_t required_length;
    required_length = std::snprintf(buffer, sizeof(buffer), "%d", number);
    if (required_length <= 0)
        return ;
    ft_size_t index;
    index = 0;
    while (index < static_cast<ft_size_t>(required_length))
    {
        target.append(buffer + index, 1);
        index += 1;
    }
    return ;
}

static ft_bool pf_engine_is_standard_conversion(char specifier)
{
    if (specifier == 'd' || specifier == 'i')
        return (FT_TRUE);
    if (specifier == 'u' || specifier == 'o')
        return (FT_TRUE);
    if (specifier == 'x' || specifier == 'X')
        return (FT_TRUE);
    if (specifier == 'f' || specifier == 'F')
        return (FT_TRUE);
    if (specifier == 'e' || specifier == 'E')
        return (FT_TRUE);
    if (specifier == 'g' || specifier == 'G')
        return (FT_TRUE);
    if (specifier == 'a' || specifier == 'A')
        return (FT_TRUE);
    if (specifier == 'c' || specifier == 's')
        return (FT_TRUE);
    if (specifier == 'p' || specifier == 'n')
        return (FT_TRUE);
    if (specifier == '%')
        return (FT_TRUE);
    if (specifier == 'b')
        return (FT_TRUE);
    return (FT_FALSE);
}

static void pf_engine_init_spec(pf_engine_format_spec &format_spec)
{
    format_spec.flag_left_align = FT_FALSE;
    format_spec.flag_force_sign = FT_FALSE;
    format_spec.flag_space_sign = FT_FALSE;
    format_spec.flag_alternate_form = FT_FALSE;
    format_spec.flag_zero_pad = FT_FALSE;
    format_spec.width_specified = FT_FALSE;
    format_spec.width_from_argument = FT_FALSE;
    format_spec.width_value = 0;
    format_spec.width_argument_index = -1;
    format_spec.precision_specified = FT_FALSE;
    format_spec.precision_from_argument = FT_FALSE;
    format_spec.precision_value = 0;
    format_spec.precision_argument_index = -1;
    format_spec.length_modifier = PF_LEN_NONE;
    format_spec.conversion_specifier = '\0';
    format_spec.argument_index = -1;
    format_spec.is_custom_specifier = FT_FALSE;
    return ;
}

static int32_t pf_engine_parse_number(const char *format, ft_size_t length, ft_size_t *cursor, int32_t *out_value)
{
    ft_size_t index;
    int32_t value;
    ft_bool has_digit;

    index = *cursor;
    value = 0;
    has_digit = FT_FALSE;
    while (index < length)
    {
        char character;
        character = format[index];
        if (!pf_engine_is_digit(character))
            break ;
        has_digit = FT_TRUE;
        int32_t digit_value;
        digit_value = character - '0';
        value = value * 10 + digit_value;
        index += 1;
    }
    if (!has_digit)
        return (FT_ERR_NOT_FOUND);
    *cursor = index;
    *out_value = value;
    return (FT_ERR_SUCCESS);
}

static int32_t pf_engine_parse_specifier(const char *format, ft_size_t length, ft_size_t *cursor, pf_engine_format_spec &format_spec, ft_bool *uses_positional, ft_bool *uses_sequential)
{
    ft_size_t index;

    pf_engine_init_spec(format_spec);
    index = *cursor;
    if (index >= length)
        return (FT_ERR_INVALID_ARGUMENT);
    ft_size_t positional_start;
    positional_start = index;
    int32_t positional_value;
    positional_value = 0;
    ft_size_t positional_cursor;
    positional_cursor = index;
    int32_t positional_parse_status;
    positional_parse_status = pf_engine_parse_number(format, length, &positional_cursor, &positional_value);
    if (positional_parse_status == FT_ERR_SUCCESS && positional_cursor < length && format[positional_cursor] == '$')
    {
        format_spec.argument_index = positional_value - 1;
        *uses_positional = FT_TRUE;
        positional_cursor += 1;
        index = positional_cursor;
    }
    else
    {
        format_spec.argument_index = -1;
        index = positional_start;
        *uses_sequential = FT_TRUE;
    }
    ft_bool parsing_flags;
    parsing_flags = FT_TRUE;
    while (parsing_flags && index < length)
    {
        char flag_character;
        flag_character = format[index];
        if (flag_character == '-')
            format_spec.flag_left_align = FT_TRUE;
        else if (flag_character == '+')
            format_spec.flag_force_sign = FT_TRUE;
        else if (flag_character == ' ')
            format_spec.flag_space_sign = FT_TRUE;
        else if (flag_character == '#')
            format_spec.flag_alternate_form = FT_TRUE;
        else if (flag_character == '0')
            format_spec.flag_zero_pad = FT_TRUE;
        else
        {
            parsing_flags = FT_FALSE;
            break ;
        }
        if (parsing_flags)
            index += 1;
    }
    if (index < length && format[index] == '*')
    {
        format_spec.width_specified = FT_TRUE;
        format_spec.width_from_argument = FT_TRUE;
        index += 1;
        ft_size_t width_cursor;
        width_cursor = index;
        int32_t width_position_value;
        width_position_value = 0;
        int32_t width_position_status;
        width_position_status = pf_engine_parse_number(format, length, &width_cursor, &width_position_value);
        if (width_position_status == FT_ERR_SUCCESS && width_cursor < length && format[width_cursor] == '$')
        {
            format_spec.width_argument_index = width_position_value - 1;
            index = width_cursor + 1;
            *uses_positional = FT_TRUE;
        }
        else
        {
            format_spec.width_argument_index = -1;
            *uses_sequential = FT_TRUE;
        }
    }
    else
    {
        int32_t width_value;
        ft_size_t width_cursor;
        width_cursor = index;
        int32_t width_status;
        width_status = pf_engine_parse_number(format, length, &width_cursor, &width_value);
        if (width_status == FT_ERR_SUCCESS)
        {
            format_spec.width_specified = FT_TRUE;
            format_spec.width_from_argument = FT_FALSE;
            format_spec.width_value = width_value;
            index = width_cursor;
        }
    }
    if (index < length && format[index] == '.')
    {
        index += 1;
        format_spec.precision_specified = FT_TRUE;
        if (index < length && format[index] == '*')
        {
            format_spec.precision_from_argument = FT_TRUE;
            format_spec.precision_value = 0;
            index += 1;
            ft_size_t precision_cursor;
            precision_cursor = index;
            int32_t precision_position_value;
            precision_position_value = 0;
            int32_t precision_position_status;
            precision_position_status = pf_engine_parse_number(format, length, &precision_cursor, &precision_position_value);
            if (precision_position_status == FT_ERR_SUCCESS && precision_cursor < length && format[precision_cursor] == '$')
            {
                format_spec.precision_argument_index = precision_position_value - 1;
                index = precision_cursor + 1;
                *uses_positional = FT_TRUE;
            }
            else
            {
                format_spec.precision_argument_index = -1;
                *uses_sequential = FT_TRUE;
            }
        }
        else
        {
            int32_t precision_value;
            ft_size_t precision_cursor;
            precision_cursor = index;
            int32_t precision_status;
            precision_status = pf_engine_parse_number(format, length, &precision_cursor, &precision_value);
            if (precision_status == FT_ERR_SUCCESS)
            {
                format_spec.precision_value = precision_value;
                index = precision_cursor;
            }
            else
                format_spec.precision_value = 0;
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
                format_spec.length_modifier = PF_LEN_HH;
                index += 2;
            }
            else
            {
                format_spec.length_modifier = PF_LEN_H;
                index += 1;
            }
        }
        else if (modifier == 'l')
        {
            if (index + 1 < length && format[index + 1] == 'l')
            {
                format_spec.length_modifier = PF_LEN_LL;
                index += 2;
            }
            else
            {
                format_spec.length_modifier = PF_LEN_L;
                index += 1;
            }
        }
        else if (modifier == 'j')
        {
            format_spec.length_modifier = PF_LEN_J;
            index += 1;
        }
        else if (modifier == 'z')
        {
            format_spec.length_modifier = PF_LEN_Z;
            index += 1;
        }
        else if (modifier == 't')
        {
            format_spec.length_modifier = PF_LEN_T;
            index += 1;
        }
        else if (modifier == 'L')
        {
            format_spec.length_modifier = PF_LEN_CAPITAL_L;
            index += 1;
        }
        else
            format_spec.length_modifier = PF_LEN_NONE;
    }
    if (index >= length)
        return (FT_ERR_INVALID_ARGUMENT);
    format_spec.conversion_specifier = format[index];
    if (!pf_engine_is_standard_conversion(format_spec.conversion_specifier))
        format_spec.is_custom_specifier = FT_TRUE;
    *cursor = index + 1;
    return (FT_ERR_SUCCESS);
}

static int32_t pf_engine_parse_format(const char *format, std::vector<pf_engine_token> &tokens, ft_bool *uses_positional, ft_bool *uses_sequential)
{
    ft_size_t length;
    length = 0;
    while (format[length])
        length += 1;
    ft_size_t index;
    index = 0;
    ft_size_t literal_start;
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
            literal_token.is_literal = FT_TRUE;
            int32_t literal_error = literal_token.ensure_literal_initialised();
            if (literal_error != FT_ERR_SUCCESS)
                return (literal_error);
            literal_token.literal_value.assign(format + literal_start, index - literal_start);
            tokens.push_back(literal_token);
        }
        index += 1;
        if (index >= length)
            return (FT_ERR_INVALID_ARGUMENT);
        if (format[index] == '%')
        {
            pf_engine_token literal_token;
            literal_token.is_literal = FT_TRUE;
            int32_t literal_error = literal_token.ensure_literal_initialised();
            if (literal_error != FT_ERR_SUCCESS)
                return (literal_error);
            literal_token.literal_value.assign("%", 1);
            tokens.push_back(literal_token);
            index += 1;
            literal_start = index;
            continue ;
        }
        pf_engine_format_spec format_spec;
        ft_size_t spec_cursor;
        spec_cursor = index;
        if (pf_engine_parse_specifier(format, length, &spec_cursor, format_spec, uses_positional, uses_sequential) != FT_ERR_SUCCESS)
            return (FT_ERR_INVALID_ARGUMENT);
        pf_engine_token spec_token;
        spec_token.is_literal = FT_FALSE;
        spec_token.format_spec = format_spec;
        int32_t literal_error = spec_token.ensure_literal_initialised();
        if (literal_error != FT_ERR_SUCCESS)
            return (literal_error);
        tokens.push_back(spec_token);
        index = spec_cursor;
        literal_start = index;
    }
    if (literal_start < length)
    {
        pf_engine_token literal_token;
        literal_token.is_literal = FT_TRUE;
        int32_t literal_error = literal_token.ensure_literal_initialised();
        if (literal_error != FT_ERR_SUCCESS)
            return (literal_error);
        literal_token.literal_value.assign(format + literal_start, length - literal_start);
        tokens.push_back(literal_token);
    }
    return (FT_ERR_SUCCESS);
}

pf_engine_token::pf_engine_token() noexcept
    : is_literal(FT_FALSE)
    , literal_value()
    , literal_initialised(FT_FALSE)
    , format_spec()
{
    return ;
}

pf_engine_token::~pf_engine_token()
{
    if (this->literal_initialised)
        (void)this->literal_value.destroy();
    return ;
}

int32_t pf_engine_token::ensure_literal_initialised() noexcept
{
    if (this->literal_initialised)
        return (FT_ERR_SUCCESS);
    int32_t initialization_error = this->literal_value.initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    this->literal_initialised = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

static int32_t pf_engine_write_literal(const ft_string &literal, t_pf_engine_write_callback writer, void *context, ft_size_t *written_count)
{
    int32_t write_error;

    if (literal.size() == 0)
        return (FT_ERR_SUCCESS);
    write_error = writer(literal.c_str(), literal.size(), context, written_count);
    if (write_error != FT_ERR_SUCCESS)
        return (write_error);
    return (FT_ERR_SUCCESS);
}

static ft_string pf_engine_build_format_string(const pf_engine_format_spec &format_spec, int32_t width_value, ft_bool width_specified, int32_t precision_value, ft_bool precision_specified)
{
    ft_string format_string;
    int32_t initialization_error = format_string.initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (format_string);
    format_string.append("%", 1);
    if (format_spec.argument_index >= 0)
    {
        pf_engine_append_signed_number(format_string, format_spec.argument_index + 1);
        format_string.append("$", 1);
    }
    if (format_spec.flag_left_align)
        format_string.append("-", 1);
    if (format_spec.flag_force_sign)
        format_string.append("+", 1);
    if (format_spec.flag_space_sign)
        format_string.append(" ", 1);
    if (format_spec.flag_alternate_form)
        format_string.append("#", 1);
    if (format_spec.flag_zero_pad && !format_spec.flag_left_align)
        format_string.append("0", 1);
    if (width_specified && width_value >= 0)
        pf_engine_append_unsigned_number(format_string, static_cast<uint32_t>(width_value));
    if (precision_specified)
    {
        format_string.append(".", 1);
        if (precision_value >= 0)
            pf_engine_append_unsigned_number(format_string, static_cast<uint32_t>(precision_value));
    }
    if (format_spec.length_modifier == PF_LEN_HH)
        format_string.append("hh", 2);
    else if (format_spec.length_modifier == PF_LEN_H)
        format_string.append("h", 1);
    else if (format_spec.length_modifier == PF_LEN_L)
        format_string.append("l", 1);
    else if (format_spec.length_modifier == PF_LEN_LL)
        format_string.append("ll", 2);
    else if (format_spec.length_modifier == PF_LEN_J)
        format_string.append("j", 1);
    else if (format_spec.length_modifier == PF_LEN_Z)
        format_string.append("z", 1);
    else if (format_spec.length_modifier == PF_LEN_T)
        format_string.append("t", 1);
    else if (format_spec.length_modifier == PF_LEN_CAPITAL_L)
        format_string.append("L", 1);
    char specifier_buffer[2];
    specifier_buffer[0] = format_spec.conversion_specifier;
    specifier_buffer[1] = '\0';
    format_string.append(specifier_buffer, 1);
    return (format_string);
}

static int32_t pf_engine_format_boolean(int32_t value, t_pf_engine_write_callback writer, void *context, ft_size_t *written_count)
{
    const char *literal;
    ft_size_t length;
    int32_t write_error;

    if (value)
    {
        literal = "FT_TRUE";
        length = 4;
    }
    else
    {
        literal = "FT_FALSE";
        length = 5;
    }
    write_error = writer(literal, length, context, written_count);
    if (write_error != FT_ERR_SUCCESS)
        return (write_error);
    return (FT_ERR_SUCCESS);
}

static int32_t pf_engine_format_with_snprintf(const ft_string &format_string, t_pf_engine_write_callback writer, void *context, ft_size_t *written_count, ...)
{
    va_list argument_list;
    int32_t write_error;

    va_start(argument_list, written_count);
    int32_t required_length;
    required_length = std::vsnprintf(ft_nullptr, 0, format_string.c_str(), argument_list);
    va_end(argument_list);
    if (required_length < 0)
        return (FT_ERR_IO);
    if (required_length == 0)
    {
        write_error = writer("", 0, context, written_count);
        if (write_error != FT_ERR_SUCCESS)
            return (write_error);
        return (FT_ERR_SUCCESS);
    }
    ft_string output;
    int32_t initialization_error = output.initialize();
    if (initialization_error != FT_ERR_SUCCESS)
        return (initialization_error);
    output.resize_length(static_cast<ft_size_t>(required_length));
    int32_t string_error = output.get_error();
    if (string_error != FT_ERR_SUCCESS)
        return (string_error);
    char *output_buffer = output.print();
    string_error = output.get_error();
    if (string_error != FT_ERR_SUCCESS)
        return (string_error);
    if (output_buffer == ft_nullptr)
        return (FT_ERR_IO);
    va_start(argument_list, written_count);
    int32_t written_length;
    written_length = std::vsnprintf(output_buffer, static_cast<ft_size_t>(required_length) + 1, format_string.c_str(), argument_list);
    va_end(argument_list);
    if (written_length < 0)
        return (FT_ERR_IO);
    output.resize_length(static_cast<ft_size_t>(written_length));
    string_error = output.get_error();
    if (string_error != FT_ERR_SUCCESS)
        return (string_error);
    const char *output_text = output.c_str();
    string_error = output.get_error();
    if (string_error != FT_ERR_SUCCESS)
        return (string_error);
    ft_size_t output_length = output.size();
    string_error = output.get_error();
    if (string_error != FT_ERR_SUCCESS)
        return (string_error);
    write_error = writer(output_text, output_length, context, written_count);
    if (write_error != FT_ERR_SUCCESS)
        return (write_error);
    return (FT_ERR_SUCCESS);
}

static int32_t pf_engine_format_standard_sequential(const pf_engine_format_spec &original_spec, va_list *argument_list, t_pf_engine_write_callback writer, void *context, ft_size_t *written_count, ft_size_t *character_count)
{
    pf_engine_format_spec format_spec;
    format_spec = original_spec;
    int32_t width_value;
    ft_bool width_specified;
    width_specified = format_spec.width_specified;
    width_value = format_spec.width_value;
    if (format_spec.width_from_argument)
    {
        int32_t dynamic_width;
        dynamic_width = va_arg(*argument_list, int32_t);
        if (dynamic_width < 0)
        {
            format_spec.flag_left_align = FT_TRUE;
            width_value = -dynamic_width;
            width_specified = FT_TRUE;
        }
        else
        {
            width_value = dynamic_width;
            width_specified = FT_TRUE;
        }
    }
    int32_t precision_value;
    ft_bool precision_specified;
    precision_specified = format_spec.precision_specified;
    precision_value = format_spec.precision_value;
    if (format_spec.precision_from_argument)
    {
        int32_t dynamic_precision;
        dynamic_precision = va_arg(*argument_list, int32_t);
        if (dynamic_precision >= 0)
        {
            precision_value = dynamic_precision;
            precision_specified = FT_TRUE;
        }
        else
            precision_specified = FT_FALSE;
    }
    if (format_spec.conversion_specifier == 'n')
    {
        ft_size_t value;
        value = *character_count;
        if (format_spec.length_modifier == PF_LEN_HH)
        {
            int8_t *pointer;
            pointer = va_arg(*argument_list, int8_t*);
            if (pointer)
                *pointer = static_cast<int8_t>(value);
        }
        else if (format_spec.length_modifier == PF_LEN_H)
        {
            int16_t *pointer;
            pointer = va_arg(*argument_list, int16_t*);
            if (pointer)
                *pointer = static_cast<int16_t>(value);
        }
        else if (format_spec.length_modifier == PF_LEN_L)
        {
            int64_t *pointer;
            pointer = va_arg(*argument_list, int64_t*);
            if (pointer)
                *pointer = static_cast<int64_t>(value);
        }
        else if (format_spec.length_modifier == PF_LEN_LL)
        {
            int64_t *pointer;
            pointer = va_arg(*argument_list, int64_t*);
            if (pointer)
                *pointer = static_cast<int64_t>(value);
        }
        else if (format_spec.length_modifier == PF_LEN_J)
        {
            int64_t *pointer;
            pointer = va_arg(*argument_list, int64_t*);
            if (pointer)
                *pointer = static_cast<int64_t>(value);
        }
        else if (format_spec.length_modifier == PF_LEN_Z)
        {
            ft_size_t *pointer;
            pointer = va_arg(*argument_list, ft_size_t*);
            if (pointer)
                *pointer = value;
        }
        else if (format_spec.length_modifier == PF_LEN_T)
        {
            int64_t *pointer;
            pointer = va_arg(*argument_list, int64_t*);
            if (pointer)
                *pointer = static_cast<int64_t>(value);
        }
        else
        {
            int32_t *pointer;
            pointer = va_arg(*argument_list, int32_t*);
            if (pointer)
                *pointer = static_cast<int32_t>(value);
        }
        return (FT_ERR_SUCCESS);
    }
    if (format_spec.conversion_specifier == 'b')
    {
        int32_t boolean_value;
        int32_t format_error;

        boolean_value = va_arg(*argument_list, int32_t);
        format_error = pf_engine_format_boolean(boolean_value, writer, context, written_count);
        if (format_error != FT_ERR_SUCCESS)
            return (format_error);
        *character_count = *written_count;
        return (FT_ERR_SUCCESS);
    }
    ft_string custom_output;
    ft_bool custom_handled;
    int32_t custom_status;

    custom_handled = FT_FALSE;
    custom_status = pf_try_format_custom_specifier(format_spec.conversion_specifier, argument_list, custom_output, &custom_handled);
    if (custom_status != FT_ERR_SUCCESS)
        return (custom_status);
    if (custom_handled == FT_TRUE)
    {
        custom_status = pf_engine_write_literal(custom_output, writer, context, written_count);
        if (custom_status != FT_ERR_SUCCESS)
            return (custom_status);
        *character_count = *written_count;
        return (FT_ERR_SUCCESS);
    }
    ft_string format_string;
    format_string = pf_engine_build_format_string(format_spec, width_value, width_specified, precision_value, precision_specified);
    int32_t status;
    if (format_spec.conversion_specifier == 'd' || format_spec.conversion_specifier == 'i')
    {
        if (format_spec.length_modifier == PF_LEN_HH)
        {
            int32_t value;
            value = va_arg(*argument_list, int32_t);
            int8_t converted;
            converted = static_cast<int8_t>(value);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, converted);
        }
        else if (format_spec.length_modifier == PF_LEN_H)
        {
            int32_t value;
            value = va_arg(*argument_list, int32_t);
            int16_t converted;
            converted = static_cast<int16_t>(value);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, converted);
        }
        else if (format_spec.length_modifier == PF_LEN_L)
        {
            int64_t value;
            value = va_arg(*argument_list, int64_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (format_spec.length_modifier == PF_LEN_LL)
        {
            int64_t value;
            value = va_arg(*argument_list, int64_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (format_spec.length_modifier == PF_LEN_J)
        {
            int64_t value;
            value = va_arg(*argument_list, int64_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (format_spec.length_modifier == PF_LEN_Z)
        {
            pf_signed_size_t value;
            value = va_arg(*argument_list, pf_signed_size_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (format_spec.length_modifier == PF_LEN_T)
        {
            int64_t value;
            value = va_arg(*argument_list, int64_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else
        {
            int32_t value;
            value = va_arg(*argument_list, int32_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
    }
    else if (format_spec.conversion_specifier == 'u' || format_spec.conversion_specifier == 'o' || format_spec.conversion_specifier == 'x' || format_spec.conversion_specifier == 'X')
    {
        if (format_spec.length_modifier == PF_LEN_HH)
        {
            uint32_t value;
            value = va_arg(*argument_list, uint32_t);
            uint8_t converted;
            converted = static_cast<uint8_t>(value);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, converted);
        }
        else if (format_spec.length_modifier == PF_LEN_H)
        {
            uint32_t value;
            value = va_arg(*argument_list, uint32_t);
            uint16_t converted;
            converted = static_cast<uint16_t>(value);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, converted);
        }
        else if (format_spec.length_modifier == PF_LEN_L)
        {
            uint64_t value;
            value = va_arg(*argument_list, uint64_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (format_spec.length_modifier == PF_LEN_LL)
        {
            uint64_t value;
            value = va_arg(*argument_list, uint64_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (format_spec.length_modifier == PF_LEN_J)
        {
            uint64_t value;
            value = va_arg(*argument_list, uint64_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (format_spec.length_modifier == PF_LEN_Z)
        {
            ft_size_t value;
            value = va_arg(*argument_list, ft_size_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else if (format_spec.length_modifier == PF_LEN_T)
        {
            pf_unsigned_ptrdiff_t value;
            value = va_arg(*argument_list, pf_unsigned_ptrdiff_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else
        {
            uint32_t value;
            value = va_arg(*argument_list, uint32_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
    }
    else if (format_spec.conversion_specifier == 'f' || format_spec.conversion_specifier == 'F' || format_spec.conversion_specifier == 'e' || format_spec.conversion_specifier == 'E' || format_spec.conversion_specifier == 'g' || format_spec.conversion_specifier == 'G' || format_spec.conversion_specifier == 'a' || format_spec.conversion_specifier == 'A')
    {
        if (format_spec.length_modifier == PF_LEN_CAPITAL_L)
        {
            long double value;
            value = va_arg(*argument_list, long double);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else
        {
            double value;
            value = va_arg(*argument_list, double);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
    }
    else if (format_spec.conversion_specifier == 'c')
    {
        if (format_spec.length_modifier == PF_LEN_L)
        {
            int32_t value;
            value = va_arg(*argument_list, int32_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else
        {
            int32_t value;
            value = va_arg(*argument_list, int32_t);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
    }
    else if (format_spec.conversion_specifier == 's')
    {
        if (format_spec.length_modifier == PF_LEN_L)
        {
            const wchar_t *value;
            value = va_arg(*argument_list, const wchar_t*);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
        else
        {
            const char *value;
            value = va_arg(*argument_list, const char*);
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
        }
    }
    else if (format_spec.conversion_specifier == 'p')
    {
        void *value;
        value = va_arg(*argument_list, void*);
        status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value);
    }
    else if (format_spec.conversion_specifier == '%')
    {
        int32_t write_error;

        write_error = writer("%", 1, context, written_count);
        if (write_error != FT_ERR_SUCCESS)
            return (write_error);
        *character_count += 1;
        return (FT_ERR_SUCCESS);
    }
    else
        return (FT_ERR_INVALID_ARGUMENT);
    if (status != FT_ERR_SUCCESS)
        return (status);
    *character_count = *written_count;
    return (FT_ERR_SUCCESS);
}

static int32_t pf_engine_process_sequential(const std::vector<pf_engine_token> &tokens, va_list argument_list, t_pf_engine_write_callback writer, void *context, ft_size_t *written_count)
{
    ft_size_t index;
    index = 0;
    ft_size_t character_count;
    character_count = *written_count;
    va_list current_args;
    va_copy(current_args, argument_list);
    int32_t status;
    status = FT_ERR_SUCCESS;
    ft_bool mark_overflow;
    mark_overflow = FT_FALSE;
    while (index < tokens.size())
    {
        const pf_engine_token &token = tokens[index];
        if (token.is_literal)
        {
            status = pf_engine_write_literal(token.literal_value, writer, context, written_count);
            if (status != FT_ERR_SUCCESS)
                break ;
            character_count = *written_count;
        }
        else
        {
            if (token.format_spec.is_custom_specifier)
            {
                ft_string custom_output;
                ft_bool handled;

                handled = FT_FALSE;
                status = pf_try_format_custom_specifier(token.format_spec.conversion_specifier, &current_args, custom_output, &handled);
                if (status != FT_ERR_SUCCESS)
                {
                    mark_overflow = FT_TRUE;
                    break ;
                }
                if (handled)
                {
                    status = pf_engine_write_literal(custom_output, writer, context, written_count);
                    if (status != FT_ERR_SUCCESS)
                        break ;
                    character_count = *written_count;
                }
                else
                {
                    status = writer("%", 1, context, written_count);
                    if (status != FT_ERR_SUCCESS)
                        break ;
                    status = writer(&token.format_spec.conversion_specifier, 1, context, written_count);
                    if (status != FT_ERR_SUCCESS)
                        break ;
                    character_count = *written_count;
                }
                index += 1;
                continue ;
            }
            status = pf_engine_format_standard_sequential(token.format_spec, &current_args, writer, context, written_count, &character_count);
            if (status != FT_ERR_SUCCESS)
                break ;
        }
        index += 1;
    }
    va_end(current_args);
    if (status != FT_ERR_SUCCESS)
    {
        if (mark_overflow == FT_TRUE)
            *written_count = SIZE_MAX;
        return (status);
    }
    *written_count = character_count;
    return (FT_ERR_SUCCESS);
}

static void pf_engine_argument_types_resize(std::vector<pf_engine_argument_kind> &kinds, ft_size_t required_index)
{
    while (kinds.size() <= required_index)
    {
        kinds.push_back(PF_ARGUMENT_NONE);
    }
}

static int32_t pf_engine_register_argument_kind(std::vector<pf_engine_argument_kind> &kinds, ft_size_t index, pf_engine_argument_kind kind)
{
    pf_engine_argument_types_resize(kinds, index);
    if (kinds[index] == PF_ARGUMENT_NONE)
    {
        kinds[index] = kind;
        return (FT_ERR_SUCCESS);
    }
    if (kinds[index] == kind)
        return (FT_ERR_SUCCESS);
    return (FT_ERR_INVALID_ARGUMENT);
}

static int32_t pf_engine_collect_argument_kinds(const std::vector<pf_engine_token> &tokens, std::vector<pf_engine_argument_kind> &kinds)
{
    ft_size_t index;
    index = 0;
    while (index < tokens.size())
    {
        const pf_engine_token &token = tokens[index];
        if (token.is_literal)
        {
            index += 1;
            continue ;
        }
        const pf_engine_format_spec &format_spec = token.format_spec;
        if (format_spec.width_from_argument && format_spec.width_argument_index >= 0)
        {
            if (pf_engine_register_argument_kind(kinds, static_cast<ft_size_t>(format_spec.width_argument_index), PF_ARGUMENT_INT) != FT_ERR_SUCCESS)
                return (FT_ERR_INVALID_ARGUMENT);
        }
        if (format_spec.precision_from_argument && format_spec.precision_argument_index >= 0)
        {
            if (pf_engine_register_argument_kind(kinds, static_cast<ft_size_t>(format_spec.precision_argument_index), PF_ARGUMENT_INT) != FT_ERR_SUCCESS)
                return (FT_ERR_INVALID_ARGUMENT);
        }
        if (format_spec.conversion_specifier == '%' || format_spec.conversion_specifier == '\0')
        {
            index += 1;
            continue ;
        }
        if (format_spec.conversion_specifier == 'n')
        {
            pf_engine_argument_kind kind;
            if (format_spec.length_modifier == PF_LEN_HH)
                kind = PF_ARGUMENT_SIGNED_CHAR_POINTER;
            else if (format_spec.length_modifier == PF_LEN_H)
                kind = PF_ARGUMENT_SHORT_POINTER;
            else if (format_spec.length_modifier == PF_LEN_L)
                kind = PF_ARGUMENT_LONG_POINTER;
            else if (format_spec.length_modifier == PF_LEN_LL)
                kind = PF_ARGUMENT_LONG_LONG_POINTER;
            else if (format_spec.length_modifier == PF_LEN_J)
                kind = PF_ARGUMENT_INTMAX_POINTER;
            else if (format_spec.length_modifier == PF_LEN_Z)
                kind = PF_ARGUMENT_SIZE_POINTER;
            else if (format_spec.length_modifier == PF_LEN_T)
                kind = PF_ARGUMENT_PTRDIFF_POINTER;
            else
                kind = PF_ARGUMENT_INT_POINTER;
            if (format_spec.argument_index < 0)
                return (FT_ERR_INVALID_ARGUMENT);
            if (pf_engine_register_argument_kind(kinds, static_cast<ft_size_t>(format_spec.argument_index), kind) != FT_ERR_SUCCESS)
                return (FT_ERR_INVALID_ARGUMENT);
            index += 1;
            continue ;
        }
        if (format_spec.conversion_specifier == 'b')
        {
            if (format_spec.argument_index < 0)
                return (FT_ERR_INVALID_ARGUMENT);
            if (pf_engine_register_argument_kind(kinds, static_cast<ft_size_t>(format_spec.argument_index), PF_ARGUMENT_BOOLEAN) != FT_ERR_SUCCESS)
                return (FT_ERR_INVALID_ARGUMENT);
            index += 1;
            continue ;
        }
        pf_engine_argument_kind kind;
        kind = PF_ARGUMENT_NONE;
        if (format_spec.conversion_specifier == 'd' || format_spec.conversion_specifier == 'i')
        {
            if (format_spec.length_modifier == PF_LEN_HH || format_spec.length_modifier == PF_LEN_H)
                kind = PF_ARGUMENT_INT;
            else if (format_spec.length_modifier == PF_LEN_L)
                kind = PF_ARGUMENT_LONG;
            else if (format_spec.length_modifier == PF_LEN_LL)
                kind = PF_ARGUMENT_LONG_LONG;
            else if (format_spec.length_modifier == PF_LEN_J)
                kind = PF_ARGUMENT_INTMAX;
            else if (format_spec.length_modifier == PF_LEN_Z)
                kind = PF_ARGUMENT_SIGNED_SIZE;
            else if (format_spec.length_modifier == PF_LEN_T)
                kind = PF_ARGUMENT_PTRDIFF;
            else
                kind = PF_ARGUMENT_INT;
        }
        else if (format_spec.conversion_specifier == 'u' || format_spec.conversion_specifier == 'o' || format_spec.conversion_specifier == 'x' || format_spec.conversion_specifier == 'X')
        {
            if (format_spec.length_modifier == PF_LEN_HH || format_spec.length_modifier == PF_LEN_H)
                kind = PF_ARGUMENT_UNSIGNED_INT;
            else if (format_spec.length_modifier == PF_LEN_L)
                kind = PF_ARGUMENT_UNSIGNED_LONG;
            else if (format_spec.length_modifier == PF_LEN_LL)
                kind = PF_ARGUMENT_UNSIGNED_LONG_LONG;
            else if (format_spec.length_modifier == PF_LEN_J)
                kind = PF_ARGUMENT_UINTMAX;
            else if (format_spec.length_modifier == PF_LEN_Z)
                kind = PF_ARGUMENT_SIZE;
            else if (format_spec.length_modifier == PF_LEN_T)
                kind = PF_ARGUMENT_UNSIGNED_PTRDIFF;
            else
                kind = PF_ARGUMENT_UNSIGNED_INT;
        }
        else if (format_spec.conversion_specifier == 'f' || format_spec.conversion_specifier == 'F' || format_spec.conversion_specifier == 'e' || format_spec.conversion_specifier == 'E' || format_spec.conversion_specifier == 'g' || format_spec.conversion_specifier == 'G' || format_spec.conversion_specifier == 'a' || format_spec.conversion_specifier == 'A')
        {
            if (format_spec.length_modifier == PF_LEN_CAPITAL_L)
                kind = PF_ARGUMENT_LONG_DOUBLE;
            else
                kind = PF_ARGUMENT_DOUBLE;
        }
        else if (format_spec.conversion_specifier == 'c')
        {
            if (format_spec.length_modifier == PF_LEN_L)
                kind = PF_ARGUMENT_WINT;
            else
                kind = PF_ARGUMENT_INT;
        }
        else if (format_spec.conversion_specifier == 's')
        {
            if (format_spec.length_modifier == PF_LEN_L)
                kind = PF_ARGUMENT_WIDE_STRING;
            else
                kind = PF_ARGUMENT_CSTRING;
        }
        else if (format_spec.conversion_specifier == 'p')
            kind = PF_ARGUMENT_VOID_POINTER;
        else
            return (FT_ERR_INVALID_ARGUMENT);
        if (format_spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        if (pf_engine_register_argument_kind(kinds, static_cast<ft_size_t>(format_spec.argument_index), kind) != FT_ERR_SUCCESS)
            return (FT_ERR_INVALID_ARGUMENT);
        index += 1;
    }
    return (FT_ERR_SUCCESS);
}

static int32_t pf_engine_store_arguments(const std::vector<pf_engine_argument_kind> &kinds, va_list argument_list, std::vector<pf_engine_argument_value> &values)
{
    ft_size_t index;
    index = 0;
    va_list current_args;
    va_copy(current_args, argument_list);
    while (index < kinds.size())
    {
        pf_engine_argument_kind kind;
        kind = kinds[index];
        pf_engine_argument_value value;
        value.kind = kind;
        if (kind == PF_ARGUMENT_INT)
            value.data.int_value = va_arg(current_args, int32_t);
        else if (kind == PF_ARGUMENT_UNSIGNED_INT)
            value.data.unsigned_int_value = va_arg(current_args, uint32_t);
        else if (kind == PF_ARGUMENT_LONG)
            value.data.long_value = va_arg(current_args, int64_t);
        else if (kind == PF_ARGUMENT_UNSIGNED_LONG)
            value.data.unsigned_long_value = va_arg(current_args, uint64_t);
        else if (kind == PF_ARGUMENT_LONG_LONG)
            value.data.long_long_value = va_arg(current_args, int64_t);
        else if (kind == PF_ARGUMENT_UNSIGNED_LONG_LONG)
            value.data.unsigned_long_long_value = va_arg(current_args, uint64_t);
        else if (kind == PF_ARGUMENT_INTMAX)
            value.data.intmax_value = va_arg(current_args, int64_t);
        else if (kind == PF_ARGUMENT_UINTMAX)
            value.data.uintmax_value = va_arg(current_args, uint64_t);
        else if (kind == PF_ARGUMENT_SIZE)
            value.data.size_value = va_arg(current_args, ft_size_t);
        else if (kind == PF_ARGUMENT_SIGNED_SIZE)
            value.data.signed_size_value = va_arg(current_args, pf_signed_size_t);
        else if (kind == PF_ARGUMENT_PTRDIFF)
            value.data.ptrdiff_value = va_arg(current_args, int64_t);
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
            value.data.wide_char_value = va_arg(current_args, int32_t);
        else if (kind == PF_ARGUMENT_INT_POINTER)
            value.data.int_pointer_value = va_arg(current_args, int32_t*);
        else if (kind == PF_ARGUMENT_LONG_POINTER)
            value.data.long_pointer_value = va_arg(current_args, int64_t*);
        else if (kind == PF_ARGUMENT_LONG_LONG_POINTER)
            value.data.long_long_pointer_value = va_arg(current_args, int64_t*);
        else if (kind == PF_ARGUMENT_SHORT_POINTER)
            value.data.short_pointer_value = va_arg(current_args, int16_t*);
        else if (kind == PF_ARGUMENT_SIGNED_CHAR_POINTER)
            value.data.signed_char_pointer_value = va_arg(current_args, int8_t*);
        else if (kind == PF_ARGUMENT_INTMAX_POINTER)
            value.data.intmax_pointer_value = va_arg(current_args, int64_t*);
        else if (kind == PF_ARGUMENT_SIZE_POINTER)
            value.data.size_pointer_value = va_arg(current_args, ft_size_t*);
        else if (kind == PF_ARGUMENT_PTRDIFF_POINTER)
            value.data.ptrdiff_pointer_value = va_arg(current_args, int64_t*);
        else if (kind == PF_ARGUMENT_BOOLEAN)
            value.data.int_value = va_arg(current_args, int32_t);
        values.push_back(value);
        index += 1;
    }
    va_end(current_args);
    return (FT_ERR_SUCCESS);
}

static int32_t pf_engine_fetch_int_argument(const std::vector<pf_engine_argument_value> &values, int32_t index, int32_t *out_value)
{
    if (index < 0)
        return (FT_ERR_INVALID_ARGUMENT);
    if (static_cast<ft_size_t>(index) >= values.size())
        return (FT_ERR_INVALID_ARGUMENT);
    const pf_engine_argument_value &value = values[static_cast<ft_size_t>(index)];
    if (value.kind != PF_ARGUMENT_INT && value.kind != PF_ARGUMENT_BOOLEAN)
        return (FT_ERR_INVALID_ARGUMENT);
    *out_value = value.data.int_value;
    return (FT_ERR_SUCCESS);
}

static int32_t pf_engine_format_standard_positional(const pf_engine_format_spec &original_spec, const std::vector<pf_engine_argument_value> &values, t_pf_engine_write_callback writer, void *context, ft_size_t *written_count, ft_size_t *character_count)
{
    pf_engine_format_spec format_spec;
    format_spec = original_spec;
    int32_t width_value;
    ft_bool width_specified;
    width_specified = format_spec.width_specified;
    width_value = format_spec.width_value;
    if (format_spec.width_from_argument && format_spec.width_argument_index >= 0)
    {
        if (pf_engine_fetch_int_argument(values, format_spec.width_argument_index, &width_value) != FT_ERR_SUCCESS)
            return (FT_ERR_INVALID_ARGUMENT);
        if (width_value < 0)
        {
            format_spec.flag_left_align = FT_TRUE;
            width_value = -width_value;
        }
        width_specified = FT_TRUE;
    }
    int32_t precision_value;
    ft_bool precision_specified;
    precision_specified = format_spec.precision_specified;
    precision_value = format_spec.precision_value;
    if (format_spec.precision_from_argument && format_spec.precision_argument_index >= 0)
    {
        if (pf_engine_fetch_int_argument(values, format_spec.precision_argument_index, &precision_value) != FT_ERR_SUCCESS)
            return (FT_ERR_INVALID_ARGUMENT);
        if (precision_value >= 0)
            precision_specified = FT_TRUE;
        else
            precision_specified = FT_FALSE;
    }
    if (format_spec.conversion_specifier == 'n')
    {
        ft_size_t value;
        value = *character_count;
        if (format_spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        pf_engine_argument_value target;
        target = values[static_cast<ft_size_t>(format_spec.argument_index)];
        if (target.kind == PF_ARGUMENT_SIGNED_CHAR_POINTER && target.data.signed_char_pointer_value)
            *target.data.signed_char_pointer_value = static_cast<int8_t>(value);
        else if (target.kind == PF_ARGUMENT_SHORT_POINTER && target.data.short_pointer_value)
            *target.data.short_pointer_value = static_cast<int16_t>(value);
        else if (target.kind == PF_ARGUMENT_LONG_POINTER && target.data.long_pointer_value)
            *target.data.long_pointer_value = static_cast<int64_t>(value);
        else if (target.kind == PF_ARGUMENT_LONG_LONG_POINTER && target.data.long_long_pointer_value)
            *target.data.long_long_pointer_value = static_cast<int64_t>(value);
        else if (target.kind == PF_ARGUMENT_INTMAX_POINTER && target.data.intmax_pointer_value)
            *target.data.intmax_pointer_value = static_cast<int64_t>(value);
        else if (target.kind == PF_ARGUMENT_SIZE_POINTER && target.data.size_pointer_value)
            *target.data.size_pointer_value = value;
        else if (target.kind == PF_ARGUMENT_PTRDIFF_POINTER && target.data.ptrdiff_pointer_value)
            *target.data.ptrdiff_pointer_value = static_cast<int64_t>(value);
        else if (target.kind == PF_ARGUMENT_INT_POINTER && target.data.int_pointer_value)
            *target.data.int_pointer_value = static_cast<int32_t>(value);
        return (FT_ERR_SUCCESS);
    }
    if (format_spec.conversion_specifier == 'b')
    {
        if (format_spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<ft_size_t>(format_spec.argument_index)];
        if (value.kind != PF_ARGUMENT_BOOLEAN)
            return (FT_ERR_INVALID_ARGUMENT);
        int32_t format_error;

        format_error = pf_engine_format_boolean(value.data.int_value, writer, context, written_count);
        if (format_error != FT_ERR_SUCCESS)
            return (format_error);
        *character_count = *written_count;
        return (FT_ERR_SUCCESS);
    }
    ft_string format_string;
    format_string = pf_engine_build_format_string(format_spec, width_value, width_specified, precision_value, precision_specified);
    int32_t status;
    status = FT_ERR_INVALID_ARGUMENT;
    if (format_spec.conversion_specifier == 'd' || format_spec.conversion_specifier == 'i')
    {
        if (format_spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<ft_size_t>(format_spec.argument_index)];
        if (format_spec.length_modifier == PF_LEN_HH)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, static_cast<int8_t>(value.data.int_value));
        else if (format_spec.length_modifier == PF_LEN_H)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, static_cast<int16_t>(value.data.int_value));
        else if (format_spec.length_modifier == PF_LEN_L)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.long_value);
        else if (format_spec.length_modifier == PF_LEN_LL)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.long_long_value);
        else if (format_spec.length_modifier == PF_LEN_J)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.intmax_value);
        else if (format_spec.length_modifier == PF_LEN_Z)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.signed_size_value);
        else if (format_spec.length_modifier == PF_LEN_T)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.ptrdiff_value);
        else
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.int_value);
    }
    else if (format_spec.conversion_specifier == 'u' || format_spec.conversion_specifier == 'o' || format_spec.conversion_specifier == 'x' || format_spec.conversion_specifier == 'X')
    {
        if (format_spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<ft_size_t>(format_spec.argument_index)];
        if (format_spec.length_modifier == PF_LEN_HH)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, static_cast<uint8_t>(value.data.unsigned_int_value));
        else if (format_spec.length_modifier == PF_LEN_H)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, static_cast<uint16_t>(value.data.unsigned_int_value));
        else if (format_spec.length_modifier == PF_LEN_L)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.unsigned_long_value);
        else if (format_spec.length_modifier == PF_LEN_LL)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.unsigned_long_long_value);
        else if (format_spec.length_modifier == PF_LEN_J)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.uintmax_value);
        else if (format_spec.length_modifier == PF_LEN_Z)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.size_value);
        else if (format_spec.length_modifier == PF_LEN_T)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.unsigned_ptrdiff_value);
        else
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.unsigned_int_value);
    }
    else if (format_spec.conversion_specifier == 'f' || format_spec.conversion_specifier == 'F' || format_spec.conversion_specifier == 'e' || format_spec.conversion_specifier == 'E' || format_spec.conversion_specifier == 'g' || format_spec.conversion_specifier == 'G' || format_spec.conversion_specifier == 'a' || format_spec.conversion_specifier == 'A')
    {
        if (format_spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<ft_size_t>(format_spec.argument_index)];
        if (format_spec.length_modifier == PF_LEN_CAPITAL_L)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.long_double_value);
        else
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.double_value);
    }
    else if (format_spec.conversion_specifier == 'c')
    {
        if (format_spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<ft_size_t>(format_spec.argument_index)];
        if (format_spec.length_modifier == PF_LEN_L)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.wide_char_value);
        else
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.int_value);
    }
    else if (format_spec.conversion_specifier == 's')
    {
        if (format_spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<ft_size_t>(format_spec.argument_index)];
        if (format_spec.length_modifier == PF_LEN_L)
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.wide_string_value);
        else
            status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.cstring_value);
    }
    else if (format_spec.conversion_specifier == 'p')
    {
        if (format_spec.argument_index < 0)
            return (FT_ERR_INVALID_ARGUMENT);
        const pf_engine_argument_value &value = values[static_cast<ft_size_t>(format_spec.argument_index)];
        status = pf_engine_format_with_snprintf(format_string, writer, context, written_count, value.data.pointer_value);
    }
    else if (format_spec.conversion_specifier == '%')
    {
        int32_t write_error;

        write_error = writer("%", 1, context, written_count);
        if (write_error != FT_ERR_SUCCESS)
            return (write_error);
        *character_count += 1;
        return (FT_ERR_SUCCESS);
    }
    if (status != FT_ERR_SUCCESS)
        return (status);
    *character_count = *written_count;
    return (FT_ERR_SUCCESS);
}

static int32_t pf_engine_process_positional(const std::vector<pf_engine_token> &tokens, va_list argument_list, t_pf_engine_write_callback writer, void *context, ft_size_t *written_count)
{
    std::vector<pf_engine_argument_kind> kinds;
    if (pf_engine_collect_argument_kinds(tokens, kinds) != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_ARGUMENT);
    std::vector<pf_engine_argument_value> values;
    if (pf_engine_store_arguments(kinds, argument_list, values) != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_ARGUMENT);
    ft_size_t index;
    index = 0;
    ft_size_t character_count;
    character_count = *written_count;
    while (index < tokens.size())
    {
        const pf_engine_token &token = tokens[index];
        if (token.is_literal)
        {
            int32_t write_error;

            write_error = pf_engine_write_literal(token.literal_value, writer, context, written_count);
            if (write_error != FT_ERR_SUCCESS)
                return (write_error);
            character_count = *written_count;
        }
        else
        {
            if (token.format_spec.is_custom_specifier)
                return (FT_ERR_INVALID_ARGUMENT);
            int32_t format_error;

            format_error = pf_engine_format_standard_positional(token.format_spec, values, writer, context, written_count, &character_count);
            if (format_error != FT_ERR_SUCCESS)
                return (format_error);
        }
        index += 1;
    }
    *written_count = character_count;
    return (FT_ERR_SUCCESS);
}

int32_t pf_engine_format(const char *format, va_list argument_list, t_pf_engine_write_callback writer, void *context, ft_size_t *written_count)
{
    try
    {
        if (!format || !written_count || !writer)
            return (FT_ERR_INVALID_ARGUMENT);
        std::vector<pf_engine_token> tokens;
        ft_bool uses_positional;
        ft_bool uses_sequential;
        uses_positional = FT_FALSE;
        uses_sequential = FT_FALSE;
        int32_t parse_status = pf_engine_parse_format(format, tokens, &uses_positional, &uses_sequential);
        if (parse_status != FT_ERR_SUCCESS)
            return (parse_status);
        if (uses_positional && uses_sequential)
        {
            return (FT_ERR_INVALID_ARGUMENT);
        }
        ft_size_t initial_count;
        initial_count = *written_count;
        int32_t status;
        if (uses_positional)
            status = pf_engine_process_positional(tokens, argument_list, writer, context, written_count);
        else
            status = pf_engine_process_sequential(tokens, argument_list, writer, context, written_count);
        if (status != FT_ERR_SUCCESS)
            return (status);
        if (*written_count < initial_count)
        {
            return (FT_ERR_INVALID_ARGUMENT);
        }
        return (FT_ERR_SUCCESS);
    }
    catch (const std::bad_alloc &)
    {
        return (FT_ERR_NO_MEMORY);
    }
    catch (...)
    {
        return (FT_ERR_INTERNAL);
    }
}
