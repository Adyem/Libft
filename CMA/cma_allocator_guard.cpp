#include <cstdlib>
#include <pthread.h>
#include <random>
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Logger/logger.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno.hpp"

static bool initialize_cma_allocator_mutex(pthread_mutex_t **mutex_storage)
{
    pthread_mutex_t *mutex_pointer;
    pthread_mutexattr_t attributes;
    bool attributes_initialized;
    int mutex_error;

    mutex_pointer = static_cast<pthread_mutex_t *>(std::malloc(sizeof(pthread_mutex_t)));
    if (mutex_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (false);
    }
    attributes_initialized = false;
    mutex_error = pthread_mutexattr_init(&attributes);
    if (mutex_error == 0)
    {
        attributes_initialized = true;
        mutex_error = pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
        if (mutex_error == 0)
            mutex_error = pthread_mutex_init(mutex_pointer, &attributes);
    }
    if (attributes_initialized)
        pthread_mutexattr_destroy(&attributes);
    if (mutex_error != 0)
    {
        mutex_error = pthread_mutex_init(mutex_pointer, ft_nullptr);
        if (mutex_error != 0)
        {
            std::free(mutex_pointer);
            ft_errno = FT_ERR_INITIALIZATION_FAILED;
            return (false);
        }
    }
    *mutex_storage = mutex_pointer;
    ft_errno = ER_SUCCESS;
    return (true);
}

static pthread_mutex_t *cma_allocator_mutex(void)
{
    static pthread_mutex_t *mutex_instance = ft_nullptr;
    static bool initialization_attempted = false;
    static bool initialized = false;

    if (!initialization_attempted)
    {
        initialization_attempted = true;
        initialized = initialize_cma_allocator_mutex(&mutex_instance);
        if (!initialized)
            mutex_instance = ft_nullptr;
    }
    if (!initialized)
        return (ft_nullptr);
    return (mutex_instance);
}

cma_allocator_guard::cma_allocator_guard()
    : _lock_acquired(false), _active(false), _error_code(ER_SUCCESS),
      _failure_logged(false), _owned_mutexes(CMA_GUARD_VECTOR_MIN_CAPACITY)
{
    if (this->acquire_allocator_mutex() == false)
    {
        this->_active = false;
        this->set_error(ft_errno);
        return ;
    }
    this->_active = true;
    this->set_error(ER_SUCCESS);
    return ;
}

cma_allocator_guard::~cma_allocator_guard()
{
    this->unlock();
    return ;
}

bool cma_allocator_guard::is_active() const
{
    if (!this->_active && !this->_failure_logged)
    {
        void *return_address;

        return_address = __builtin_return_address(0);
        this->log_inactive_guard(return_address);
    }
    return (this->_active);
}

bool cma_allocator_guard::lock_acquired() const
{
    return (this->_lock_acquired);
}

void cma_allocator_guard::unlock()
{
    int entry_errno;
    int release_error;

    if (!this->_active)
        return ;
    entry_errno = ft_errno;
    release_error = this->release_all_mutexes();
    this->_active = false;
    this->set_error(release_error);
    if (release_error == ER_SUCCESS)
        ft_errno = entry_errno;
    return ;
}

int cma_allocator_guard::get_error() const
{
    return (this->_error_code);
}

const char *cma_allocator_guard::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

void cma_allocator_guard::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

void cma_allocator_guard::log_inactive_guard(void *return_address) const
{
    pt_thread_id_type thread_identifier;
    int previous_errno;

    if (this->_failure_logged)
        return ;
    this->_failure_logged = true;
    previous_errno = ft_errno;
    thread_identifier = pt_thread_self();
    ft_log_error("cma_allocator_guard inactive thread=%p return_address=%p",
        reinterpret_cast<void *>(thread_identifier), return_address);
    ft_errno = previous_errno;
    return ;
}

bool cma_allocator_guard::acquire_allocator_mutex()
{
    pthread_mutex_t *mutex_pointer;

    mutex_pointer = cma_allocator_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->_lock_acquired = false;
        this->set_error(ft_errno);
        return (false);
    }
    return (this->acquire_mutex(mutex_pointer));
}

bool cma_allocator_guard::acquire_mutex(pthread_mutex_t *mutex_pointer)
{
    pt_thread_id_type thread_identifier;
    bool local_lock_acquired;
    cma_guard_vector<s_mutex_entry> previous_mutexes;
    pt_mutex_vector current_mutexes;
    pt_mutex_vector tracked_mutexes;
    ft_size_t index;

    thread_identifier = pt_thread_self();
    while (true)
    {
        current_mutexes = pt_lock_tracking::get_owned_mutexes(thread_identifier);
        if (ft_errno != ER_SUCCESS)
        {
            this->_lock_acquired = false;
            this->set_error(ft_errno);
            return (false);
        }
        tracked_mutexes = this->owned_mutex_pointers();
        index = 0;
        while (index < tracked_mutexes.size())
        {
            if (!this->mutex_vector_contains(current_mutexes, tracked_mutexes[index]))
                current_mutexes.push_back(tracked_mutexes[index]);
            index += 1;
        }
        if (!pt_lock_tracking::notify_wait(thread_identifier, mutex_pointer, current_mutexes))
        {
            int release_error;
            int snapshot_error;

            pt_lock_tracking::notify_released(thread_identifier, mutex_pointer);
            previous_mutexes = this->snapshot_owned_mutexes();
            snapshot_error = previous_mutexes.get_error();
            if (snapshot_error != ER_SUCCESS)
            {
                release_error = this->release_all_mutexes();
                if (release_error != ER_SUCCESS)
                    this->set_error(release_error);
                else
                    this->set_error(snapshot_error);
                return (false);
            }
            release_error = this->release_all_mutexes();
            if (release_error != ER_SUCCESS)
            {
                this->set_error(release_error);
                return (false);
            }
            this->sleep_random_backoff();
            if (!this->reacquire_mutexes(previous_mutexes))
            {
                int reacquire_error;

                reacquire_error = ft_errno;
                release_error = this->release_all_mutexes();
                if (release_error != ER_SUCCESS)
                    this->set_error(release_error);
                else
                    this->set_error(reacquire_error);
                return (false);
            }
            continue ;
        }
        local_lock_acquired = false;
        if (cma_lock_allocator(&local_lock_acquired) != 0)
        {
            pt_lock_tracking::notify_released(thread_identifier, mutex_pointer);
            this->_lock_acquired = false;
            this->set_error(ft_errno);
            return (false);
        }
        if (!local_lock_acquired)
        {
            pt_lock_tracking::notify_released(thread_identifier, mutex_pointer);
            this->_lock_acquired = false;
            this->set_error(FT_ERR_INVALID_STATE);
            return (false);
        }
        this->_lock_acquired = true;
        if (!this->track_mutex_acquisition(mutex_pointer, local_lock_acquired))
        {
            int vector_error;
            int unlock_error;

            vector_error = this->_owned_mutexes.get_error();
            cma_unlock_allocator(local_lock_acquired);
            unlock_error = ft_errno;
            pt_lock_tracking::notify_released(thread_identifier, mutex_pointer);
            this->_lock_acquired = false;
            if (unlock_error != ER_SUCCESS)
                this->set_error(unlock_error);
            else
                this->set_error(vector_error);
            return (false);
        }
        pt_lock_tracking::notify_acquired(thread_identifier, mutex_pointer);
        this->set_error(ER_SUCCESS);
        return (true);
    }
}

int cma_allocator_guard::release_all_mutexes()
{
    pt_thread_id_type thread_identifier;
    ft_size_t count;
    int release_error;

    thread_identifier = pt_thread_self();
    release_error = ER_SUCCESS;
    count = this->_owned_mutexes.size();
    while (count > 0)
    {
        pthread_mutex_t *mutex_pointer;
        bool lock_state;

        count -= 1;
        mutex_pointer = this->_owned_mutexes[count].mutex_pointer;
        lock_state = this->_owned_mutexes[count].lock_acquired;
        cma_unlock_allocator(lock_state);
        if (ft_errno != ER_SUCCESS && release_error == ER_SUCCESS)
            release_error = ft_errno;
        pt_lock_tracking::notify_released(thread_identifier, mutex_pointer);
        if (ft_errno != ER_SUCCESS && release_error == ER_SUCCESS)
            release_error = ft_errno;
        this->_owned_mutexes.pop_back();
    }
    this->_lock_acquired = false;
    if (release_error != ER_SUCCESS)
        ft_errno = release_error;
    else
        ft_errno = ER_SUCCESS;
    return (release_error);
}

bool cma_allocator_guard::reacquire_mutexes(const cma_guard_vector<s_mutex_entry> &previous_mutexes)
{
    ft_size_t index;

    index = 0;
    while (index < previous_mutexes.size())
    {
        if (!this->acquire_mutex(previous_mutexes[index].mutex_pointer))
            return (false);
        index += 1;
    }
    return (true);
}

bool cma_allocator_guard::track_mutex_acquisition(pthread_mutex_t *mutex_pointer, bool lock_acquired)
{
    ft_size_t index;
    s_mutex_entry entry;

    index = 0;
    while (index < this->_owned_mutexes.size())
    {
        if (this->_owned_mutexes[index].mutex_pointer == mutex_pointer)
        {
            this->_owned_mutexes[index].lock_acquired = lock_acquired;
            return (true);
        }
        index += 1;
    }
    entry.mutex_pointer = mutex_pointer;
    entry.lock_acquired = lock_acquired;
    this->_owned_mutexes.push_back(entry);
    if (this->_owned_mutexes.get_error() != ER_SUCCESS)
    {
        this->_lock_acquired = false;
        this->set_error(this->_owned_mutexes.get_error());
        return (false);
    }
    return (true);
}

pt_mutex_vector cma_allocator_guard::owned_mutex_pointers() const
{
    pt_mutex_vector mutex_pointers;
    ft_size_t index;

    index = 0;
    while (index < this->_owned_mutexes.size())
    {
        mutex_pointers.push_back(this->_owned_mutexes[index].mutex_pointer);
        index += 1;
    }
    return (mutex_pointers);
}

cma_guard_vector<cma_allocator_guard::s_mutex_entry> cma_allocator_guard::snapshot_owned_mutexes() const
{
    cma_guard_vector<s_mutex_entry> snapshot;
    ft_size_t index;

    index = 0;
    while (index < this->_owned_mutexes.size())
    {
        snapshot.push_back(this->_owned_mutexes[index]);
        if (snapshot.get_error() != ER_SUCCESS)
            return (snapshot);
        index += 1;
    }
    return (snapshot);
}

bool cma_allocator_guard::mutex_vector_contains(const pt_mutex_vector &mutexes, pthread_mutex_t *mutex_pointer) const
{
    ft_size_t index;

    index = 0;
    while (index < mutexes.size())
    {
        if (mutexes[index] == mutex_pointer)
            return (true);
        index += 1;
    }
    return (false);
}

void cma_allocator_guard::sleep_random_backoff() const
{
    static thread_local bool generator_initialized = false;
    static thread_local std::minstd_rand generator;
    std::uniform_int_distribution<int> distribution(1, 10);
    std::random_device device;
    int delay_ms;

    if (!generator_initialized)
    {
        generator.seed(device());
        generator_initialized = true;
    }
    delay_ms = distribution(generator);
    pt_thread_sleep(static_cast<unsigned int>(delay_ms));
    return ;
}

int cma_lock_allocator(bool *lock_acquired)
{
    bool guard_incremented;
    int mutex_error;
    pthread_mutex_t *mutex_pointer;

    if (!lock_acquired)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    *lock_acquired = false;
    mutex_pointer = cma_allocator_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_INITIALIZATION_FAILED;
        return (-1);
    }
    mutex_error = pthread_mutex_lock(mutex_pointer);
    if (mutex_error != 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    if (cma_metadata_make_writable() != 0)
    {
        pthread_mutex_unlock(mutex_pointer);
        return (-1);
    }
    guard_incremented = cma_metadata_guard_increment();
    if (!guard_incremented)
    {
        pthread_mutex_unlock(mutex_pointer);
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void cma_unlock_allocator(bool lock_acquired)
{
    bool guard_decremented;
    int mutex_error;
    pthread_mutex_t *mutex_pointer;

    if (!lock_acquired)
    {
        ft_errno = ER_SUCCESS;
        return ;
    }
    guard_decremented = cma_metadata_guard_decrement();
    mutex_pointer = cma_allocator_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_INITIALIZATION_FAILED;
        return ;
    }
    mutex_error = pthread_mutex_unlock(mutex_pointer);
    if (!guard_decremented)
        return ;
    if (mutex_error != 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return ;
    }
    ft_errno = ER_SUCCESS;
    return ;
}

