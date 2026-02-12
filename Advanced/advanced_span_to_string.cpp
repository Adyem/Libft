#include "advanced.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <new>

ft_string *adv_span_to_string(const char *buffer, ft_size_t length)
{
    if (buffer == ft_nullptr && length != 0)
        return (ft_nullptr);
    ft_string *result = new (std::nothrow) ft_string();
    if (result == ft_nullptr)
        return (ft_nullptr);
    if (result->initialize() != FT_ERR_SUCCESS)
    {
        delete result;
        return (ft_nullptr);
    }
    if (length == 0)
        return (result);
    if (result->append(buffer, length) != FT_ERR_SUCCESS)
    {
        delete result;
        return (ft_nullptr);
    }
    return (result);
}
