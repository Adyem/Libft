#include "event_emitter.hpp"
#include <cstdint>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

template class ft_event_emitter<int32_t>;
