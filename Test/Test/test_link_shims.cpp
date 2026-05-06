#include "../test_internal.hpp"
#include "../../Modules/PThread/condition.hpp"
#include "../../Modules/PThread/mutex.hpp"

int pt_condition_variable::wait(pt_mutex &mutex)
{
    struct timespec wait_duration;

    wait_duration.tv_sec = 3600;
    wait_duration.tv_nsec = 0;
    return (this->wait_for(mutex, wait_duration));
}
