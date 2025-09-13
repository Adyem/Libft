#include "compatebility_internal.hpp"
#include "../System_utils/system_utils.hpp"

ssize_t cmp_su_write(int file_descriptor, const char *buffer, size_t length)
{
#if defined(_WIN32) || defined(_WIN64)
    return (su_write(file_descriptor, buffer, static_cast<unsigned int>(length)));
#else
    return (su_write(file_descriptor, buffer, length));
#endif
}
