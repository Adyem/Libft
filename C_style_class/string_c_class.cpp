#include "string_c_class.hpp"

namespace str
{
	char *dereference(str::string string, int index)
	{
		if (index < 0 || index >= string._size)
		{
			static char null_terminator = '\0';
			return (&null_terminator);
		}
		return (&string._data[index]);
	}
}
