#ifndef FT_THREAD_POOL_HPP
#define FT_THREAD_POOL_HPP

#include "constructor.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"

#include "vector.hpp"
#include "queue.hpp"
#include "../PThread/thread.hpp"
#include "function.hpp"
#include <cstddef>
#include <cerrno>
#include <pthread.h>
#include "../PThread/pthread.hpp"
#include <utility>
#include <atomic>

class ft_thread_pool
{
    private:
        ft_vector<ft_thread>          _workers;
        ft_queue<ft_function<void()> > _tasks;
        size_t                        _max_tasks;
        bool                          _stop;
        size_t                        _active;
        mutable std::atomic<int>        _error_code;
        pthread_mutex_t               _mutex;
        pthread_cond_t                _cond;
        bool                          _mutex_initialized;
        bool                          _cond_initialized;

        void set_error(int error) const;
        void worker();

    public:
        ft_thread_pool(size_t thread_count = 0, size_t max_tasks = 0);
        ~ft_thread_pool();

        ft_thread_pool(const ft_thread_pool&) = delete;
        ft_thread_pool& operator=(const ft_thread_pool&) = delete;
        ft_thread_pool(ft_thread_pool&&) = delete;
        ft_thread_pool& operator=(ft_thread_pool&&) = delete;

        template <typename Function>
        void submit(Function &&function);

        void wait();
        void destroy();

        int get_error() const;
        const char* get_error_str() const;
};

inline void ft_thread_pool::worker()
{
    while (true)
    {
        ft_function<void()> task;
        if (pthread_mutex_lock(&this->_mutex) != 0)
        {
            this->set_error(errno + ERRNO_OFFSET);
            return ;
        }
        bool queue_empty;
        queue_empty = this->_tasks.empty();
        if (this->_tasks.get_error() != ER_SUCCESS)
        {
            pthread_mutex_unlock(&this->_mutex);
            this->set_error(this->_tasks.get_error());
            continue;
        }
        while (queue_empty && !this->_stop)
        {
            if (pt_cond_wait(&this->_cond, &this->_mutex) != 0)
            {
                pthread_mutex_unlock(&this->_mutex);
                this->set_error(ft_errno);
                return ;
            }
            queue_empty = this->_tasks.empty();
            if (this->_tasks.get_error() != ER_SUCCESS)
            {
                pthread_mutex_unlock(&this->_mutex);
                this->set_error(this->_tasks.get_error());
                return ;
            }
        }
        if (this->_stop && queue_empty)
        {
            pthread_mutex_unlock(&this->_mutex);
            return ;
        }
        task = this->_tasks.dequeue();
        if (this->_tasks.get_error() != ER_SUCCESS)
        {
            pthread_mutex_unlock(&this->_mutex);
            this->set_error(this->_tasks.get_error());
            continue;
        }
        ++this->_active;
        pthread_mutex_unlock(&this->_mutex);
        task();
        if (pthread_mutex_lock(&this->_mutex) != 0)
        {
            this->set_error(errno + ERRNO_OFFSET);
            return ;
        }
        --this->_active;
        queue_empty = this->_tasks.empty();
        if (this->_tasks.get_error() != ER_SUCCESS)
        {
            pthread_mutex_unlock(&this->_mutex);
            this->set_error(this->_tasks.get_error());
            return ;
        }
        if (queue_empty && this->_active == 0)
            pt_cond_broadcast(&this->_cond);
        pthread_mutex_unlock(&this->_mutex);
    }
}

inline void ft_thread_pool::set_error(int error) const
{
    this->_error_code.store(error, std::memory_order_relaxed);
    ft_errno = error;
}

inline ft_thread_pool::ft_thread_pool(size_t thread_count, size_t max_tasks)
    : _workers(), _tasks(), _max_tasks(max_tasks), _stop(false), _active(0), _error_code(ER_SUCCESS),
      _mutex(), _cond(), _mutex_initialized(false), _cond_initialized(false)
{
    if (pthread_mutex_init(&this->_mutex, ft_nullptr) != 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        this->_stop = true;
        return ;
    }
    this->_mutex_initialized = true;
    if (pt_cond_init(&this->_cond, ft_nullptr) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->_cond_initialized = true;
    this->_workers.reserve(thread_count);
    if (this->_workers.get_error() != ER_SUCCESS)
    {
        this->set_error(THREAD_POOL_ALLOC_FAIL);
        this->_stop = true;
        return ;
    }
    size_t worker_index = 0;
    while (worker_index < thread_count)
    {
        ft_thread worker(&ft_thread_pool::worker, this);
        this->_workers.push_back(std::move(worker));
        if (this->_workers.get_error() != ER_SUCCESS)
        {
            this->set_error(THREAD_POOL_ALLOC_FAIL);
            this->_stop = true;
            break;
        }
        ++worker_index;
    }
}

inline ft_thread_pool::~ft_thread_pool()
{
    this->destroy();
    if (this->_cond_initialized)
        pt_cond_destroy(&this->_cond);
    if (this->_mutex_initialized)
        pthread_mutex_destroy(&this->_mutex);
}

template <typename Function>
inline void ft_thread_pool::submit(Function &&function)
{
    if (pthread_mutex_lock(&this->_mutex) != 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return ;
    }
    if (this->_stop)
    {
        pthread_mutex_unlock(&this->_mutex);
        return ;
    }
    if (this->_max_tasks != 0 && this->_tasks.size() >= this->_max_tasks)
    {
        this->set_error(THREAD_POOL_FULL);
        pthread_mutex_unlock(&this->_mutex);
        return ;
    }
    int queue_error;
    bool enqueue_failed;

    enqueue_failed = false;
    {
        ft_function<void()> wrapper(std::move(function));

        this->_tasks.enqueue(std::move(wrapper));
        queue_error = this->_tasks.get_error();
        if (queue_error != ER_SUCCESS)
        {
            wrapper = ft_function<void()>();
            enqueue_failed = true;
        }
    }
    if (enqueue_failed)
    {
        this->set_error(queue_error);
        pthread_mutex_unlock(&this->_mutex);
        ft_errno = queue_error;
        return ;
    }
    this->set_error(ER_SUCCESS);
    pthread_mutex_unlock(&this->_mutex);
    pt_cond_signal(&this->_cond);
}

inline void ft_thread_pool::wait()
{
    if (pthread_mutex_lock(&this->_mutex) != 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return ;
    }
    bool tasks_empty;
    tasks_empty = this->_tasks.empty();
    if (this->_tasks.get_error() != ER_SUCCESS)
    {
        pthread_mutex_unlock(&this->_mutex);
        this->set_error(this->_tasks.get_error());
        return ;
    }
    while (!tasks_empty || this->_active != 0)
    {
        if (pt_cond_wait(&this->_cond, &this->_mutex) != 0)
        {
            pthread_mutex_unlock(&this->_mutex);
            this->set_error(ft_errno);
            return ;
        }
        tasks_empty = this->_tasks.empty();
        if (this->_tasks.get_error() != ER_SUCCESS)
        {
            pthread_mutex_unlock(&this->_mutex);
            this->set_error(this->_tasks.get_error());
            return ;
        }
    }
    pthread_mutex_unlock(&this->_mutex);
    this->set_error(ER_SUCCESS);
}

inline void ft_thread_pool::destroy()
{
    if (pthread_mutex_lock(&this->_mutex) != 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return ;
    }
    this->_stop = true;
    pthread_mutex_unlock(&this->_mutex);
    pt_cond_broadcast(&this->_cond);
    size_t worker_index = 0;
    while (worker_index < this->_workers.size())
    {
        if (this->_workers[worker_index].joinable())
            this->_workers[worker_index].join();
        ++worker_index;
    }
    this->_workers.clear();
    this->set_error(ER_SUCCESS);
}

inline int ft_thread_pool::get_error() const
{
    int error_value;

    error_value = this->_error_code.load(std::memory_order_relaxed);
    ft_errno = error_value;
    return (error_value);
}

inline const char* ft_thread_pool::get_error_str() const
{
      return (ft_strerror(this->_error_code.load(std::memory_order_relaxed)));
}

#endif 
