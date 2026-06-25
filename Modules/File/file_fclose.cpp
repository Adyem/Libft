#include "../Basic/config.hpp"
#include "file_utils.hpp"
#include "../Basic/class_nullptr.hpp"
#include <cstdio>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t ft_fclose(FILE *stream)
{
    if (stream == ft_nullptr)
        return (EOF);
    if (std::fclose(stream) != 0)
        return (EOF);
    return (FT_ERR_SUCCESS);
}
