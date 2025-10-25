#ifndef GEOMETRY_LOCK_TRACKER_HPP
# define GEOMETRY_LOCK_TRACKER_HPP

#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

int     geometry_lock_tracker_lock_pair(const void *first_object, const void *second_object,
            pt_mutex &first_mutex, pt_mutex &second_mutex,
            ft_unique_lock<pt_mutex> &first_guard,
            ft_unique_lock<pt_mutex> &second_guard);

#endif
