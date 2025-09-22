#include "pthread.hpp"

int pt_atomic_fetch_add(ft_atomic<int>& atomic_variable, int increment_value)
{
    return (atomic_variable.fetch_add(increment_value));
}

