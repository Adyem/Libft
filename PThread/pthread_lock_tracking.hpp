#ifndef PTHREAD_LOCK_TRACKING_HPP
# define PTHREAD_LOCK_TRACKING_HPP

#include <pthread.h>
#include <vector>
#include <cstddef>
#include <cstdlib>
#include <new>
#include "pthread.hpp"
#include "../CPP_class/class_nullptr.hpp"

template <typename t_type>
class pt_system_allocator
{
    public:
        typedef t_type value_type;

        pt_system_allocator();
        template <typename t_other>
        pt_system_allocator(const pt_system_allocator<t_other> &other);
        ~pt_system_allocator();

        t_type *allocate(std::size_t count);
        void deallocate(t_type *pointer, std::size_t count);

        template <typename t_other>
        struct rebind
        {
            typedef pt_system_allocator<t_other> other;
        };
};

template <typename t_type, typename t_other>
bool operator==(const pt_system_allocator<t_type> &left, const pt_system_allocator<t_other> &right);

template <typename t_type, typename t_other>
bool operator!=(const pt_system_allocator<t_type> &left, const pt_system_allocator<t_other> &right);

typedef std::vector<pthread_mutex_t *, pt_system_allocator<pthread_mutex_t *> > pt_mutex_vector;
typedef std::vector<pt_thread_id_type, pt_system_allocator<pt_thread_id_type> > pt_thread_vector;

struct s_pt_lock_wait_snapshot
{
    pthread_mutex_t    *mutex_pointer;
    pt_thread_id_type  owner_thread;
    pt_thread_id_type  waiting_thread;
    long               wait_started_ms;
};

typedef std::vector<s_pt_lock_wait_snapshot, pt_system_allocator<s_pt_lock_wait_snapshot> > pt_lock_wait_snapshot_vector;

struct s_pt_thread_lock_info
{
    pt_thread_id_type thread_identifier;
    pt_mutex_vector owned_mutexes;
    pthread_mutex_t *waiting_mutex;
    long wait_started_ms;
};

#ifdef PT_LOCK_TRACKING_TESTING
struct s_pt_lock_tracking_thread_state
{
    pt_thread_id_type thread_identifier;
    pt_mutex_vector owned_mutexes;
    pthread_mutex_t *waiting_mutex;
    long wait_started_ms;
};
#endif

class pt_lock_tracking
{
    private:
        static pthread_mutex_t *get_registry_mutex(void);
        static std::vector<s_pt_thread_lock_info, pt_system_allocator<s_pt_thread_lock_info> > *get_thread_infos(void);
        static bool ensure_registry_mutex_initialized(void);

        static s_pt_thread_lock_info *find_thread_info(pt_thread_id_type thread_identifier);
        static s_pt_thread_lock_info *lookup_thread_info(pt_thread_id_type thread_identifier);
        static bool vector_contains_mutex(const pt_mutex_vector &mutexes, pthread_mutex_t *mutex_pointer);
        static bool vector_contains_thread(const pt_thread_vector &thread_identifiers, pt_thread_id_type thread_identifier);
        static bool detect_cycle(const s_pt_thread_lock_info *origin, pthread_mutex_t *requested_mutex, pt_mutex_vector *visited_mutexes, pt_thread_vector *visited_threads);

    public:
        pt_lock_tracking();
        ~pt_lock_tracking();

        static pt_mutex_vector get_owned_mutexes(pt_thread_id_type thread_identifier);
        static bool notify_wait(pt_thread_id_type thread_identifier, pthread_mutex_t *requested_mutex, const pt_mutex_vector &owned_mutexes);
        static void notify_acquired(pt_thread_id_type thread_identifier, pthread_mutex_t *mutex_pointer);
        static void notify_released(pt_thread_id_type thread_identifier, pthread_mutex_t *mutex_pointer);
        static bool snapshot_waiters(pt_lock_wait_snapshot_vector &snapshot);
#ifdef PT_LOCK_TRACKING_TESTING
        static bool get_thread_state(pt_thread_id_type thread_identifier, s_pt_lock_tracking_thread_state &state);
#endif
};

#include "pthread_lock_tracking.tpp"

#endif
