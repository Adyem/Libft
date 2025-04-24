#include <cstdlib>
#include "../CPP_class/string_class.hpp"
#include "open_dir.hpp"

#if defined(_WIN32) || defined(_WIN64)
  #include <windows.h>
  #include <algorithm>
  #define PATH_SEP '\\'
#else
  #include <sys/stat.h>
  #include <unistd.h>
  #define PATH_SEP '/'
#endif

static ft_string normalize_path(ft_string path)
{
	#if defined(_WIN32) || defined(_WIN64)
    	std::replace(path.begin(), path.end(), '/', PATH_SEP);
	#endif
    	return (path);
}

int dir_exists(const char *rel_path)
{
    ft_string path = normalize_path(rel_path);
	if (path.getError())
		return (-1);
	#if defined(_WIN32) || defined(_WIN64)
	    DWORD attr = GetFileAttributesA(path.c_str());
	    if (attr != INVALID_FILE_ATTRIBUTES &&
	        (attr & FILE_ATTRIBUTE_DIRECTORY))
	        return 1;
	#else
    	struct stat st;
    	if (stat(path.c_str(), &st) == 0 &&
    	    S_ISDIR(st.st_mode))
    	    return (1);
	#endif
		return (0);
}

