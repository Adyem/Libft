#include "api_internal.hpp"
#include "../Errno/errno.hpp"

ft_bool api_append_content_length_header(ft_string &request, ft_size_t content_length)
{
    char content_length_buffer[32];
    char reversed_buffer[32];
    ft_size_t reversed_length;
    ft_size_t index;
    int32_t request_error;

    if (!request.is_initialised())
    {
        if (request.initialize() != FT_ERR_SUCCESS)
            return (FT_FALSE);
    }

    reversed_length = 0;
    do
    {
        if (reversed_length >= sizeof(reversed_buffer) - 1)
            return (FT_FALSE);
        reversed_buffer[reversed_length] = static_cast<char>('0' + (content_length % 10));
        content_length /= 10;
        reversed_length++;
    }
    while (content_length != 0);
    index = 0;
    while (index < reversed_length)
    {
        ft_size_t reversed_index;

        reversed_index = reversed_length - index - 1;
        content_length_buffer[index] = reversed_buffer[reversed_index];
        index++;
    }
    content_length_buffer[reversed_length] = '\0';
    request += "\r\nContent-Length: ";
    request_error = ft_string::get_error();
    if (request_error != FT_ERR_SUCCESS)
    {
        return (FT_FALSE);
    }
    request += content_length_buffer;
    request_error = ft_string::get_error();
    if (request_error != FT_ERR_SUCCESS)
    {
        return (FT_FALSE);
    }
    return (FT_TRUE);
}
