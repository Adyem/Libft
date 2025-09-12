#include "file_utils.hpp"

#ifdef _WIN32
# include <windows.h>

int file_copy(const char *source_path, const char *destination_path)
{
    if (CopyFileA(source_path, destination_path, 0))
        return (0);
    return (-1);
}

#else
# include <filesystem>

int file_copy(const char *source_path, const char *destination_path)
{
    std::error_code copy_error_code;
    std::filesystem::copy_file(source_path, destination_path, std::filesystem::copy_options::overwrite_existing, copy_error_code);
    if (copy_error_code.value() == 0)
        return (0);
    return (-1);
}
#endif
