#include "future.hpp"
#include <cstdint>
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "promise.hpp"
#include "shared_ptr.hpp"

template class ft_future<int32_t>;
