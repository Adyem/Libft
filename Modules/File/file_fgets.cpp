#include "../Basic/config.hpp"
#include "file_utils.hpp"
#include "../Basic/class_nullptr.hpp"
#include <cstdio>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

char *ft_fgets(char *string, int32_t size, FILE *stream)
{
    if (string == ft_nullptr || stream == ft_nullptr || size <= 0)
        return (ft_nullptr);
    return (std::fgets(string, size, stream));
}
