#include "pool.hpp"
#include <cstdint>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "vector.hpp"

template class Pool<int32_t>;
