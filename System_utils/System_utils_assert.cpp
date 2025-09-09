#include "../Logger/logger_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "system_utils.hpp"
#include <cstdlib>

void su_assert(bool condition, const char *message)
{
    if (condition)
        return ;
    if (g_logger != ft_nullptr)
        g_logger->error("Assertion failed: %s", message);
    abort();
}
