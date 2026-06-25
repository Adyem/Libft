#include "uri_internal.hpp"
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/limits.hpp"

static char *uri_normalize_percent_text(const char *input, ft_size_t input_size,
    ft_bool keep_slash)
{
    char *output;
    ft_size_t input_index;
    ft_size_t output_index;
    int32_t high_value;
    int32_t low_value;
    uint8_t decoded_value;

    if (uri_mul_overflows(input_size, 3) == FT_TRUE)
        return (ft_nullptr);
    output = static_cast<char *>(cma_malloc((input_size * 3) + 1));
    if (output == ft_nullptr)
        return (ft_nullptr);
    input_index = 0;
    output_index = 0;
    while (input_index < input_size)
    {
        if (input[input_index] == '%' && input_index + 2 < input_size)
        {
            high_value = uri_hex_value(input[input_index + 1]);
            low_value = uri_hex_value(input[input_index + 2]);
            if (high_value >= 0 && low_value >= 0)
            {
                decoded_value = static_cast<uint8_t>(
                        (high_value << 4) | low_value);
                if (uri_is_unreserved(decoded_value) == FT_TRUE
                    || (keep_slash == FT_TRUE && decoded_value == '/'))
                    output[output_index++] = static_cast<char>(decoded_value);
                else
                {
                    output[output_index++] = '%';
                    output[output_index++] = uri_hex_character(
                            static_cast<uint8_t>(decoded_value >> 4));
                    output[output_index++] = uri_hex_character(
                            static_cast<uint8_t>(decoded_value & 0x0FU));
                }
                input_index += 3;
            }
            else
                output[output_index++] = input[input_index++];
        }
        else
            output[output_index++] = input[input_index++];
    }
    output[output_index] = '\0';
    return (output);
}

static ft_bool uri_segment_is_dot(const char *path, ft_size_t start,
    ft_size_t end) noexcept
{
    if (end - start == 1 && path[start] == '.')
        return (FT_TRUE);
    if (end - start == 3 && path[start] == '%' && path[start + 1] == '2'
        && (path[start + 2] == 'e' || path[start + 2] == 'E'))
        return (FT_TRUE);
    return (FT_FALSE);
}

static ft_bool uri_segment_is_dot_dot(const char *path, ft_size_t start,
    ft_size_t end) noexcept
{
    if (end - start == 2 && path[start] == '.' && path[start + 1] == '.')
        return (FT_TRUE);
    return (FT_FALSE);
}

static ft_size_t uri_remove_previous_segment(char *output,
    ft_size_t output_index) noexcept
{
    if (output_index == 0)
        return (0);
    if (output_index > 1 && output[output_index - 1] == '/')
        output_index--;
    while (output_index > 0 && output[output_index - 1] != '/')
        output_index--;
    if (output_index > 1)
        output_index--;
    return (output_index);
}

static char *uri_remove_dot_segments(const char *path)
{
    char *output;
    ft_size_t path_size;
    ft_size_t input_index;
    ft_size_t segment_end;
    ft_size_t output_index;
    ft_bool leading_slash;

    path_size = ft_strlen_size_t(path);
    output = static_cast<char *>(cma_malloc(path_size + 1));
    if (output == ft_nullptr)
        return (ft_nullptr);
    input_index = 0;
    output_index = 0;
    while (input_index < path_size)
    {
        leading_slash = FT_FALSE;
        if (path[input_index] == '/')
        {
            leading_slash = FT_TRUE;
            input_index++;
        }
        segment_end = input_index;
        while (segment_end < path_size && path[segment_end] != '/')
            segment_end++;
        if (uri_segment_is_dot(path, input_index, segment_end) == FT_TRUE)
        {
            if (leading_slash == FT_TRUE && output_index == 0)
                output[output_index++] = '/';
        }
        else if (uri_segment_is_dot_dot(path, input_index, segment_end)
            == FT_TRUE)
            output_index = uri_remove_previous_segment(output, output_index);
        else
        {
            if (leading_slash == FT_TRUE)
                output[output_index++] = '/';
            while (input_index < segment_end)
                output[output_index++] = path[input_index++];
        }
        input_index = segment_end;
    }
    output[output_index] = '\0';
    return (output);
}

static ft_size_t uri_normalized_length(const uri_components *components,
    const char *path) noexcept
{
    ft_size_t length;

    length = uri_component_length(components->scheme);
    if (components->scheme != ft_nullptr)
        length += 1;
    if (components->host != ft_nullptr)
        length += 2;
    length += uri_component_length(components->userinfo);
    if (components->userinfo != ft_nullptr)
        length += 1;
    length += uri_component_length(components->host);
    if (components->port != ft_nullptr)
        length += 1 + uri_component_length(components->port);
    length += uri_component_length(path);
    if (components->query != ft_nullptr)
        length += 1 + uri_component_length(components->query);
    if (components->fragment != ft_nullptr)
        length += 1 + uri_component_length(components->fragment);
    return (length);
}

static char *uri_build_normalized(const uri_components *components,
    const char *path)
{
    char *output;
    ft_size_t output_index;
    ft_size_t length;

    length = uri_normalized_length(components, path);
    output = static_cast<char *>(cma_malloc(length + 1));
    if (output == ft_nullptr)
        return (ft_nullptr);
    output_index = 0;
    uri_append_text(output, &output_index, components->scheme);
    if (components->scheme != ft_nullptr)
        output[output_index++] = ':';
    if (components->host != ft_nullptr)
    {
        output[output_index++] = '/';
        output[output_index++] = '/';
    }
    uri_append_text(output, &output_index, components->userinfo);
    if (components->userinfo != ft_nullptr)
        output[output_index++] = '@';
    uri_append_text(output, &output_index, components->host);
    if (components->port != ft_nullptr)
    {
        output[output_index++] = ':';
        uri_append_text(output, &output_index, components->port);
    }
    uri_append_text(output, &output_index, path);
    if (components->query != ft_nullptr)
    {
        output[output_index++] = '?';
        uri_append_text(output, &output_index, components->query);
    }
    if (components->fragment != ft_nullptr)
    {
        output[output_index++] = '#';
        uri_append_text(output, &output_index, components->fragment);
    }
    output[output_index] = '\0';
    return (output);
}

char *uri_normalize(const char *input, ft_size_t input_size)
{
    uri_components components;
    char *normalized_path;
    char *clean_path;
    char *output;

    if (uri_parse(input, input_size, &components) != FT_ERR_SUCCESS)
        return (ft_nullptr);
    uri_lower_string(components.scheme);
    uri_lower_string(components.host);
    clean_path = uri_normalize_percent_text(components.path,
            ft_strlen_size_t(components.path), FT_TRUE);
    if (clean_path == ft_nullptr)
    {
        uri_components_destroy(&components);
        uri_set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    normalized_path = uri_remove_dot_segments(clean_path);
    cma_free(clean_path);
    if (normalized_path == ft_nullptr)
    {
        uri_components_destroy(&components);
        uri_set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    output = uri_build_normalized(&components, normalized_path);
    cma_free(normalized_path);
    uri_components_destroy(&components);
    if (output == ft_nullptr)
    {
        uri_set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    uri_set_error(FT_ERR_SUCCESS);
    return (output);
}
