#include "pthread.hpp"

void pt_atomic_store(ft_atomic<int>& atomic_variable, int desired_value)
{
    atomic_variable.store(desired_value);
    return ;
}

