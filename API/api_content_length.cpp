#include "api_internal.hpp"
#include "../Errno/errno.hpp"

bool api_append_content_length_header(ft_string &request, size_t content_length)
{
    char content_length_buffer[32];
    char reversed_buffer[32];
    size_t reversed_length;
    size_t index;
    int request_error;

    reversed_length = 0;
    do
    {
        if (reversed_length >= sizeof(reversed_buffer) - 1)
            return (false);
        reversed_buffer[reversed_length] = static_cast<char>('0' + (content_length % 10));
        content_length /= 10;
        reversed_length++;
    }
    while (content_length != 0);
    index = 0;
    while (index < reversed_length)
    {
        size_t reversed_index;

        reversed_index = reversed_length - index - 1;
        content_length_buffer[index] = reversed_buffer[reversed_index];
        index++;
    }
    content_length_buffer[reversed_length] = '\0';
    request += "\r\nContent-Length: ";
    request_error = request.get_error();
    if (request_error != FT_ER_SUCCESSS)
    {
        ft_errno = request_error;
        return (false);
    }
    request += content_length_buffer;
    request_error = request.get_error();
    if (request_error != FT_ER_SUCCESSS)
    {
        ft_errno = request_error;
        return (false);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (true);
}
