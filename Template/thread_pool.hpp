#ifndef FT_THREAD_POOL_HPP
#define FT_THREAD_POOL_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/condition.hpp"
#include "../PThread/recursive_mutex.hpp"
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
        size_t                          _configured_thread_count;
        size_t                          _max_tasks;
        bool                            _stop;
        size_t                          _active;
        pthread_mutex_t                 _mutex;
        pthread_cond_t                  _cond;
        bool                            _mutex_initialized;
        bool                            _cond_initialized;
        mutable pt_recursive_mutex     *_thread_safe_mutex;
        uint8_t                         _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        static thread_local int32_t _last_error;

        static int32_t set_last_operation_error(int32_t error_code) noexcept
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

        void abort_if_not_initialized(const char *method_name) const
        {
            if (this->_initialized_state == _state_initialized)
                return ;
            this->abort_lifecycle_error(method_name,
                "called while object is not initialized");
            return ;
        }

        int lock_internal(bool *lock_acquired) const
        {
            int lock_result;

            if (lock_acquired != ft_nullptr)
                *lock_acquired = false;
            if (this->_thread_safe_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            lock_result = this->_thread_safe_mutex->lock();
            if (lock_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_result));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = true;
            return (FT_ERR_SUCCESS);
        }

        void unlock_internal(bool lock_acquired) const
        {
            if (lock_acquired == false)
                return ;
            if (this->_thread_safe_mutex == ft_nullptr)
                return ;
            (void)this->_thread_safe_mutex->unlock();
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
                while (this->_stop == false && this->_tasks.empty())
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
        }

    public:
        ft_thread_pool(size_t thread_count = 0, size_t max_tasks = 0)
            : _workers(), _tasks(), _configured_thread_count(thread_count),
              _max_tasks(max_tasks), _stop(false), _active(0), _mutex(), _cond(),
              _mutex_initialized(false), _cond_initialized(false),
              _thread_safe_mutex(ft_nullptr), _initialized_state(_state_uninitialized)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_thread_pool()
        {
            if (this->_initialized_state == _state_uninitialized)
                this->abort_lifecycle_error("ft_thread_pool::~ft_thread_pool",
                    "destructor called while object is uninitialized");
            if (this->_initialized_state == _state_initialized)
                this->destroy();
            if (this->_thread_safe_mutex != ft_nullptr)
                this->disable_thread_safety();
            return ;
        }

        ft_thread_pool(const ft_thread_pool&) = delete;
        ft_thread_pool& operator=(const ft_thread_pool&) = delete;
        ft_thread_pool(ft_thread_pool&&) = delete;
        ft_thread_pool& operator=(ft_thread_pool&&) = delete;

        int initialize()
        {
            size_t worker_index;

            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("ft_thread_pool::initialize",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_stop = false;
            this->_active = 0;
            if (pthread_mutex_init(&this->_mutex, ft_nullptr) != 0)
            {
                this->_initialized_state = _state_destroyed;
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_mutex_initialized = true;
            if (pthread_cond_init(&this->_cond, ft_nullptr) != 0)
            {
                pthread_mutex_destroy(&this->_mutex);
                this->_mutex_initialized = false;
                this->_initialized_state = _state_destroyed;
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_cond_initialized = true;
            this->_workers.clear();
            this->_tasks.initialize();
            worker_index = 0;
            while (worker_index < this->_configured_thread_count)
            {
                ft_thread worker(&ft_thread_pool::worker_entry, this);

                this->_workers.push_back(ft_move(worker));
                ++worker_index;
            }
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int initialize(size_t thread_count, size_t max_tasks)
        {
            this->_configured_thread_count = thread_count;
            this->_max_tasks = max_tasks;
            return (this->initialize());
        }

        void destroy()
        {
            size_t worker_index;
            size_t worker_count;

            this->abort_if_not_initialized("ft_thread_pool::destroy");
            if (pthread_mutex_lock(&this->_mutex) != 0)
            {
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return ;
            }
            this->_stop = true;
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
            if (this->_cond_initialized)
            {
                pthread_cond_destroy(&this->_cond);
                this->_cond_initialized = false;
            }
            if (this->_mutex_initialized)
            {
                pthread_mutex_destroy(&this->_mutex);
                this->_mutex_initialized = false;
            }
            this->_initialized_state = _state_destroyed;
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        template <typename Function>
        void submit(Function &&function)
        {
            bool lock_acquired;
            int lock_error;
            size_t task_count;

            this->abort_if_not_initialized("ft_thread_pool::submit");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            if (pthread_mutex_lock(&this->_mutex) != 0)
            {
                this->unlock_internal(lock_acquired);
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return ;
            }
            if (this->_stop)
            {
                pthread_mutex_unlock(&this->_mutex);
                this->unlock_internal(lock_acquired);
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return ;
            }
            if (this->_max_tasks != 0)
            {
                task_count = this->_tasks.size();
                if (task_count >= this->_max_tasks)
                {
                    pthread_mutex_unlock(&this->_mutex);
                    this->unlock_internal(lock_acquired);
                    set_last_operation_error(FT_ERR_FULL);
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
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        template <typename Function>
        void submit(Function &&function, const ft_cancellation_token &token)
        {
            if (token.is_cancellation_requested())
            {
                set_last_operation_error(FT_ERR_SUCCESS);
                return ;
            }
            this->submit(std::forward<Function>(function));
            return ;
        }

        void wait()
        {
            bool lock_acquired;
            int lock_error;

            this->abort_if_not_initialized("ft_thread_pool::wait");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            if (pthread_mutex_lock(&this->_mutex) != 0)
            {
                this->unlock_internal(lock_acquired);
                set_last_operation_error(FT_ERR_INVALID_STATE);
                return ;
            }
            while (this->_tasks.empty() == false || this->_active != 0)
                pthread_cond_wait(&this->_cond, &this->_mutex);
            pthread_mutex_unlock(&this->_mutex);
            this->unlock_internal(lock_acquired);
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        int enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int initialize_result;

            this->abort_if_not_initialized("ft_thread_pool::enable_thread_safety");
            if (this->_thread_safe_mutex != ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            new_mutex = new (std::nothrow) pt_recursive_mutex();
            if (new_mutex == ft_nullptr)
                return (set_last_operation_error(FT_ERR_NO_MEMORY));
            initialize_result = new_mutex->initialize();
            if (initialize_result != FT_ERR_SUCCESS)
            {
                delete new_mutex;
                return (set_last_operation_error(initialize_result));
            }
            this->_thread_safe_mutex = new_mutex;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int disable_thread_safety()
        {
            pt_recursive_mutex *mutex_pointer;
            int destroy_result;

            if (this->_initialized_state != _state_initialized
                && this->_initialized_state != _state_destroyed)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            mutex_pointer = this->_thread_safe_mutex;
            if (mutex_pointer == ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            this->_thread_safe_mutex = ft_nullptr;
            destroy_result = mutex_pointer->destroy();
            delete mutex_pointer;
            if (destroy_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(destroy_result));
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        bool is_thread_safe() const
        {
            this->abort_if_not_initialized("ft_thread_pool::is_thread_safe");
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_thread_safe_mutex != ft_nullptr);
        }

        int lock(bool *lock_acquired) const
        {
            int lock_result;

            this->abort_if_not_initialized("ft_thread_pool::lock");
            lock_result = this->lock_internal(lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (-1);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (0);
        }

        void unlock(bool lock_acquired) const
        {
            this->unlock_internal(lock_acquired);
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        static int32_t last_operation_error() noexcept
        {
            return (_last_error);
        }

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept
        {
            return (this->_thread_safe_mutex);
        }
#endif
};

thread_local int32_t ft_thread_pool::_last_error = FT_ERR_SUCCESS;

#endif
