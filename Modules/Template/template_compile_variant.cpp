#include "variant.hpp"
#include <cstdint>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

template class ft_variant<int32_t, const char *>;
