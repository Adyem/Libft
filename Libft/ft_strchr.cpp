#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"

char	*ft_strchr(const char *string, int char_to_find)
{
	char	target_char;

	if (!string)
		return (ft_nullptr);
	target_char = static_cast<char>(char_to_find);
	while (*string)
	{
		if (*string == target_char)
			return (const_cast<char *>(string));
		string++;
	}
	if (target_char == '\0')
		return (const_cast<char *>(string));
	return (ft_nullptr);
}
