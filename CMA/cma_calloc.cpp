#include <cstddef>
#include <stdbool.h>
#include "CMA.hpp"

void	*cma_calloc(std::size_t count, std::size_t size)
{
	void			*ptr;
	std::size_t		total_size;
	char			*char_ptr;

	if (count <= 0 || size <= 0)
		return (NULL);
	total_size = count * size;
	ptr = cma_malloc(total_size);
	if (!ptr)
		return (NULL);
	char_ptr = static_cast<char*>(ptr);
	std::size_t index = 0;
	while (index < total_size)
	{
		char_ptr[index] = 0;
		index++;
	}
	return (ptr);
}
