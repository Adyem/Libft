#include "uri_internal.hpp"
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"

static thread_local int32_t g_uri_error = FT_ERR_SUCCESS;

int32_t uri_set_error(int32_t error_code) noexcept
{
    g_uri_error = error_code;
    return (error_code);
}

int32_t uri_get_error(void) noexcept
{
    return (g_uri_error);
}

const char *uri_get_error_str(void) noexcept
{
    return (ft_strerror(g_uri_error));
}

ft_bool uri_mul_overflows(ft_size_t left, ft_size_t right) noexcept
{
    if (left != 0 && right > FT_SYSTEM_SIZE_MAX / left)
        return (FT_TRUE);
    return (FT_FALSE);
}

ft_bool uri_is_alpha(char character) noexcept
{
    if (character >= 'A' && character <= 'Z')
        return (FT_TRUE);
    if (character >= 'a' && character <= 'z')
        return (FT_TRUE);
    return (FT_FALSE);
}

ft_bool uri_is_digit(char character) noexcept
{
    if (character >= '0' && character <= '9')
        return (FT_TRUE);
    return (FT_FALSE);
}

ft_bool uri_is_scheme_character(char character) noexcept
{
    if (uri_is_alpha(character) == FT_TRUE)
        return (FT_TRUE);
    if (uri_is_digit(character) == FT_TRUE)
        return (FT_TRUE);
    if (character == '+' || character == '-' || character == '.')
        return (FT_TRUE);
    return (FT_FALSE);
}

ft_bool uri_is_unreserved(uint8_t value) noexcept
{
    if (value >= 'A' && value <= 'Z')
        return (FT_TRUE);
    if (value >= 'a' && value <= 'z')
        return (FT_TRUE);
    if (value >= '0' && value <= '9')
        return (FT_TRUE);
    if (value == '-' || value == '_' || value == '.' || value == '~')
        return (FT_TRUE);
    return (FT_FALSE);
}

char uri_lower_character(char character) noexcept
{
    if (character >= 'A' && character <= 'Z')
        return (static_cast<char>(character + ('a' - 'A')));
    return (character);
}

char uri_hex_character(uint8_t value) noexcept
{
    if (value < 10)
        return (static_cast<char>('0' + value));
    return (static_cast<char>('A' + (value - 10)));
}

int32_t uri_hex_value(char character) noexcept
{
    if (character >= '0' && character <= '9')
        return (character - '0');
    if (character >= 'a' && character <= 'f')
        return (character - 'a' + 10);
    if (character >= 'A' && character <= 'F')
        return (character - 'A' + 10);
    return (-1);
}

char *uri_duplicate_span(const char *input, ft_size_t start,
    ft_size_t end) noexcept
{
    char *output;
    ft_size_t index;
    ft_size_t size;

    if (end < start)
        return (ft_nullptr);
    size = end - start;
    output = static_cast<char *>(cma_malloc(size + 1));
    if (output == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < size)
    {
        output[index] = input[start + index];
        index++;
    }
    output[size] = '\0';
    return (output);
}

int32_t uri_assign_component(char **destination, const char *input,
    ft_size_t start, ft_size_t end) noexcept
{
    *destination = uri_duplicate_span(input, start, end);
    if (*destination == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    return (FT_ERR_SUCCESS);
}

void uri_lower_string(char *string) noexcept
{
    ft_size_t index;

    if (string == ft_nullptr)
        return ;
    index = 0;
    while (string[index] != '\0')
    {
        string[index] = uri_lower_character(string[index]);
        index++;
    }
    return ;
}

ft_size_t uri_component_length(const char *component) noexcept
{
    if (component == ft_nullptr)
        return (0);
    return (ft_strlen_size_t(component));
}

void uri_append_text(char *output, ft_size_t *output_index,
    const char *text) noexcept
{
    ft_size_t index;

    if (text == ft_nullptr)
        return ;
    index = 0;
    while (text[index] != '\0')
        output[(*output_index)++] = text[index++];
    return ;
}

void uri_delete_components(uri_components *components) noexcept
{
    if (components == ft_nullptr)
        return ;
    uri_components_destroy(components);
    return ;
}
