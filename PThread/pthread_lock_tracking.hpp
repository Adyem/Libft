#ifndef PTHREAD_LOCK_TRACKING_HPP
# define PTHREAD_LOCK_TRACKING_HPP

#include <pthread.h>
#include <vector>
#include "pthread.hpp"
#include "../CPP_class/class_nullptr.hpp"

struct s_pt_thread_lock_info
{
    pt_thread_id_type thread_identifier;
    std::vector<pthread_mutex_t *> owned_mutexes;
    pthread_mutex_t *waiting_mutex;
};

class pt_lock_tracking
{
    private:
        static pthread_mutex_t _registry_mutex;
        static std::vector<s_pt_thread_lock_info> _thread_infos;

        static s_pt_thread_lock_info *find_thread_info(pt_thread_id_type thread_identifier);
        static bool vector_contains_mutex(const std::vector<pthread_mutex_t *> &mutexes, pthread_mutex_t *mutex_pointer);
        static bool vector_contains_thread(const std::vector<pt_thread_id_type> &thread_identifiers, pt_thread_id_type thread_identifier);
        static bool detect_cycle(const s_pt_thread_lock_info *origin, pthread_mutex_t *requested_mutex, std::vector<pthread_mutex_t *> *visited_mutexes, std::vector<pt_thread_id_type> *visited_threads);

    public:
        pt_lock_tracking();
        ~pt_lock_tracking();

        static std::vector<pthread_mutex_t *> get_owned_mutexes(pt_thread_id_type thread_identifier);
        static bool notify_wait(pt_thread_id_type thread_identifier, pthread_mutex_t *requested_mutex, const std::vector<pthread_mutex_t *> &owned_mutexes);
        static void notify_acquired(pt_thread_id_type thread_identifier, pthread_mutex_t *mutex_pointer);
        static void notify_released(pt_thread_id_type thread_identifier, pthread_mutex_t *mutex_pointer);
};

#endif
