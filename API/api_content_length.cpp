#include "api_internal.hpp"
#include "../Printf/printf.hpp"

bool api_append_content_length_header(ft_string &request, size_t content_length)
{
    char content_length_buffer[32];
    int formatted_length;

    formatted_length = pf_snprintf(content_length_buffer, sizeof(content_length_buffer), "%zu", content_length);
    if (formatted_length < 0)
        return (false);
    if (static_cast<size_t>(formatted_length) >= sizeof(content_length_buffer))
        return (false);
    request += "\r\nContent-Length: ";
    request += content_length_buffer;
    return (true);
}
