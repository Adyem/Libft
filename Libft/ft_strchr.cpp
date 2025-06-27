#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"

char	*ft_strchr(const char *string, int char_to_find)
{
	char	target_char;
	char	*last_occurrence;

	if (!string)
		return (ft_nullptr);
    if (!string)
        return (ft_nullptr);
    target_char = static_cast<char>(char_to_find);
    last_occurrence = ft_nullptr;
    while (*string)
	{
        if (*string == target_char)
            last_occurrence = const_cast<char *>(string);
        string++;
	}
    if (target_char == '\0')
        return (const_cast<char *>(string));
    return (last_occurrence);
}
