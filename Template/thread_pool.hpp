#ifndef FT_THREAD_POOL_HPP
#define FT_THREAD_POOL_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/condition.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/thread.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "vector.hpp"
#include "queue.hpp"
#include "cancellation.hpp"
#include "function.hpp"
#include "move.hpp"
#include <cstddef>
#include <pthread.h>
#include <utility>
#include <type_traits>

class ft_thread_pool
{
    private:
        ft_vector<ft_thread>            _workers;
        ft_queue<ft_function<void()> >  _tasks;
        ft_size_t                          _configured_thread_count;
        ft_size_t                          _max_tasks;
        bool                            _stop;
        ft_size_t                          _active;
        pthread_mutex_t                 _mutex;
        pthread_cond_t                  _cond;
        bool                            _mutex_initialised;
        bool                            _cond_initialised;
        mutable pt_recursive_mutex     *_thread_safe_mutex;
        uint8_t                         _initialised_state;

        static const uint8_t _state_uninitialised = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialised = 2;
        static thread_local int32_t _last_error;

        static int32_t set_error(int32_t error_code) noexcept
        {
            _last_error = error_code;
            return (error_code);
        }

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const
        {
            if (method_name == ft_nullptr)
                method_name = "unknown";
            if (reason == ft_nullptr)
                reason = "unknown";
            pf_printf_fd(2, "ft_thread_pool lifecycle error: %s: %s\n", method_name,
                reason);
            su_abort();
            return ;
        }

        void abort_if_not_initialised(const char *method_name) const
        {
            if (this->_initialised_state == _state_initialised)
                return ;
            this->abort_lifecycle_error(method_name,
                "called while object is not initialised");
            return ;
        }

        int32_t lock_internal(ft_bool *lock_acquired) const
        {
            int32_t lock_result;

            if (lock_acquired != ft_nullptr)
                *lock_acquired = FT_FALSE;
            if (this->_thread_safe_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            lock_result = pt_recursive_mutex_lock_if_not_null(this->_thread_safe_mutex);
            if (lock_result != FT_ERR_SUCCESS)
                return (set_error(lock_result));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = FT_TRUE;
            return (FT_ERR_SUCCESS);
        }

        void unlock_internal(ft_bool lock_acquired) const
        {
            if (lock_acquired == FT_FALSE)
                return ;
            if (this->_thread_safe_mutex == ft_nullptr)
                return ;
            (void)pt_recursive_mutex_unlock_if_not_null(this->_thread_safe_mutex);
            return ;
        }

        static void worker_entry(ft_thread_pool *pool)
        {
            if (pool == ft_nullptr)
                return ;
            pool->worker();
            return ;
        }

        void worker()
        {
            while (true)
            {
                ft_function<void()> task;

                if (pthread_mutex_lock(&this->_mutex) != 0)
                    return ;
                while (this->_stop == FT_FALSE && this->_tasks.empty())
                    pthread_cond_wait(&this->_cond, &this->_mutex);
                if (this->_stop && this->_tasks.empty())
                {
                    pthread_mutex_unlock(&this->_mutex);
                    return ;
                }
                task = this->_tasks.dequeue();
                this->_active += 1;
                pthread_mutex_unlock(&this->_mutex);
                task();
                pthread_mutex_lock(&this->_mutex);
                this->_active -= 1;
                if (this->_tasks.empty() && this->_active == 0)
                    pthread_cond_broadcast(&this->_cond);
                pthread_mutex_unlock(&this->_mutex);
            }
            return ;
        }

    public:
        ft_thread_pool(ft_size_t thread_count = 0, ft_size_t max_tasks = 0)
            : _workers(), _tasks(), _configured_thread_count(thread_count),
              _max_tasks(max_tasks), _stop(false), _active(0), _mutex(), _cond(),
              _mutex_initialised(false), _cond_initialised(false),
              _thread_safe_mutex(ft_nullptr), _initialised_state(_state_uninitialised)
        {
            return ;
        }

        ~ft_thread_pool()
        {
            int32_t previous_error;

            previous_error = _last_error;
            if (this->_initialised_state == _state_initialised)
                (void)this->destroy();
            if (this->_thread_safe_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            (void)set_error(previous_error);
            return ;
        }

        ft_thread_pool(const ft_thread_pool&) = delete;
        ft_thread_pool& operator=(const ft_thread_pool&) = delete;
        ft_thread_pool(ft_thread_pool&&) = delete;
        ft_thread_pool& operator=(ft_thread_pool&&) = delete;

        int32_t initialize()
        {
            ft_size_t worker_index;

            if (this->_initialised_state == _state_initialised)
            {
                this->abort_lifecycle_error("ft_thread_pool::initialize",
                    "called while object is already initialised");
                return (set_error(FT_ERR_INVALID_STATE));
            }
            this->_stop = FT_FALSE;
            this->_active = 0;
            if (pthread_mutex_init(&this->_mutex, ft_nullptr) != 0)
            {
                this->_initialised_state = _state_destroyed;
                return (set_error(FT_ERR_INVALID_STATE));
            }
            this->_mutex_initialised = FT_TRUE;
            if (pthread_cond_init(&this->_cond, ft_nullptr) != 0)
            {
                pthread_mutex_destroy(&this->_mutex);
                this->_mutex_initialised = FT_FALSE;
                this->_initialised_state = _state_destroyed;
                return (set_error(FT_ERR_INVALID_STATE));
            }
            this->_cond_initialised = FT_TRUE;
            this->_workers.clear();
            this->_tasks.initialize();
            worker_index = 0;
            while (worker_index < this->_configured_thread_count)
            {
                ft_thread worker(&ft_thread_pool::worker_entry, this);

                this->_workers.push_back(ft_move(worker));
                ++worker_index;
            }
            this->_initialised_state = _state_initialised;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t initialize(ft_size_t thread_count, ft_size_t max_tasks)
        {
            this->_configured_thread_count = thread_count;
            this->_max_tasks = max_tasks;
            return (this->initialize());
        }

        int32_t destroy()
        {
            ft_size_t worker_index;
            ft_size_t worker_count;

            if (this->_initialised_state != _state_initialised)
            {
                return (set_error(FT_ERR_SUCCESS));
            }
            if (pthread_mutex_lock(&this->_mutex) != 0)
            {
                return (set_error(FT_ERR_INVALID_STATE));
            }
            this->_stop = FT_TRUE;
            pthread_mutex_unlock(&this->_mutex);
            pthread_cond_broadcast(&this->_cond);
            worker_index = 0;
            worker_count = this->_workers.size();
            while (worker_index < worker_count)
            {
                if (this->_workers[worker_index].joinable())
                    this->_workers[worker_index].join();
                ++worker_index;
            }
            this->_workers.clear();
            this->_tasks.clear();
            if (this->_cond_initialised)
            {
                pthread_cond_destroy(&this->_cond);
                this->_cond_initialised = FT_FALSE;
            }
            if (this->_mutex_initialised)
            {
                pthread_mutex_destroy(&this->_mutex);
                this->_mutex_initialised = FT_FALSE;
            }
            this->_initialised_state = _state_destroyed;
            return (set_error(FT_ERR_SUCCESS));
        }

        template <typename Function>
        void submit(Function &&function)
        {
            ft_bool lock_acquired;
            int32_t lock_error;
            ft_size_t task_count;

            this->abort_if_not_initialised("ft_thread_pool::submit");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return ;
            }
            if (pthread_mutex_lock(&this->_mutex) != 0)
            {
                this->unlock_internal(lock_acquired);
                set_error(FT_ERR_INVALID_STATE);
                return ;
            }
            if (this->_stop)
            {
                pthread_mutex_unlock(&this->_mutex);
                this->unlock_internal(lock_acquired);
                set_error(FT_ERR_INVALID_STATE);
                return ;
            }
            if (this->_max_tasks != 0)
            {
                task_count = this->_tasks.size();
                if (task_count >= this->_max_tasks)
                {
                    pthread_mutex_unlock(&this->_mutex);
                    this->unlock_internal(lock_acquired);
                    set_error(FT_ERR_FULL);
                    return ;
                }
            }
            {
                ft_function<void()> wrapper(ft_move(function));
                this->_tasks.enqueue(ft_move(wrapper));
            }
            pthread_cond_signal(&this->_cond);
            pthread_mutex_unlock(&this->_mutex);
            this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        template <typename Function>
        void submit(Function &&function, const ft_cancellation_token &token)
        {
            if (token.is_cancellation_requested())
            {
                set_error(FT_ERR_SUCCESS);
                return ;
            }
            this->submit(std::forward<Function>(function));
            return ;
        }

        void wait()
        {
            ft_bool lock_acquired;
            int32_t lock_error;

            this->abort_if_not_initialised("ft_thread_pool::wait");
            lock_acquired = FT_FALSE;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_error(lock_error);
                return ;
            }
            if (pthread_mutex_lock(&this->_mutex) != 0)
            {
                this->unlock_internal(lock_acquired);
                set_error(FT_ERR_INVALID_STATE);
                return ;
            }
            while (this->_tasks.empty() == FT_FALSE || this->_active != 0)
                pthread_cond_wait(&this->_cond, &this->_mutex);
            pthread_mutex_unlock(&this->_mutex);
            this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        int32_t enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int32_t initialize_result;

            this->abort_if_not_initialised("ft_thread_pool::enable_thread_safety");
            if (this->_thread_safe_mutex != ft_nullptr)
                return (set_error(FT_ERR_SUCCESS));
            new_mutex = new (std::nothrow) pt_recursive_mutex();
            if (new_mutex == ft_nullptr)
                return (set_error(FT_ERR_NO_MEMORY));
            initialize_result = new_mutex->initialize();
            if (initialize_result != FT_ERR_SUCCESS)
            {
                delete new_mutex;
                return (set_error(initialize_result));
            }
            this->_thread_safe_mutex = new_mutex;
            return (set_error(FT_ERR_SUCCESS));
        }

        int32_t disable_thread_safety()
        {
            pt_recursive_mutex *mutex_pointer;
            int32_t destroy_result;

            if (this->_initialised_state != _state_initialised
                && this->_initialised_state != _state_destroyed)
                return (set_error(FT_ERR_INVALID_STATE));
            mutex_pointer = this->_thread_safe_mutex;
            if (mutex_pointer == ft_nullptr)
                return (set_error(FT_ERR_SUCCESS));
            this->_thread_safe_mutex = ft_nullptr;
            destroy_result = mutex_pointer->destroy();
            delete mutex_pointer;
            if (destroy_result != FT_ERR_SUCCESS)
                return (set_error(destroy_result));
            return (set_error(FT_ERR_SUCCESS));
        }

        bool is_thread_safe() const
        {
            this->abort_if_not_initialised("ft_thread_pool::is_thread_safe");
            set_error(FT_ERR_SUCCESS);
            return (this->_thread_safe_mutex != ft_nullptr);
        }

        int32_t lock(ft_bool *lock_acquired) const
        {
            int32_t lock_result;

            this->abort_if_not_initialised("ft_thread_pool::lock");
            lock_result = this->lock_internal(lock_acquired);
            return (set_error(lock_result));
        }

        void unlock(ft_bool lock_acquired) const
        {
            this->unlock_internal(lock_acquired);
            set_error(FT_ERR_SUCCESS);
            return ;
        }

        static int32_t get_error() noexcept
        {
            return (_last_error);
        }

};

thread_local int32_t ft_thread_pool::_last_error = FT_ERR_SUCCESS;

#endif
