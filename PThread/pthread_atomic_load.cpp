#include "pthread.hpp"

int pt_atomic_load(const ft_atomic<int>& atomic_variable)
{
    return (atomic_variable.load());
}

