#include "libft.hpp"
#include <sstream>

ft_string ft_to_string(long number)
{
    std::ostringstream stream;
    stream << number;
    ft_string number_string(stream.str().c_str());
    return (number_string);
}
