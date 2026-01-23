#ifndef GEOMETRY_LOCK_TRACKER_HPP
# define GEOMETRY_LOCK_TRACKER_HPP

#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int     geometry_lock_tracker_lock_pair(const void *first_object, const void *second_object,
            pt_recursive_mutex &first_mutex, pt_recursive_mutex &second_mutex);

#endif
