#ifndef FT_THREAD_POOL_HPP
#define FT_THREAD_POOL_HPP

#include "constructor.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <cstddef>
#include <atomic>

class ft_thread_pool
{
    private:
        std::vector<std::thread>      _workers;
        std::queue<std::function<void()> > _tasks;
        size_t                        _max_tasks;
        bool                          _stop;
        size_t                        _active;
        mutable std::atomic<int>      _error_code;
        std::mutex                    _mutex;
        std::condition_variable       _cond;

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
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(this->_mutex);
            this->_cond.wait(lock, [this]() { return (this->_stop || !this->_tasks.empty()); });
            if (this->_stop && this->_tasks.empty())
                return ;
            task = std::move(this->_tasks.front());
            this->_tasks.pop();
            ++this->_active;
        }
        task();
        {
            std::unique_lock<std::mutex> lock(this->_mutex);
            --this->_active;
            if (this->_tasks.empty() && this->_active == 0)
                this->_cond.notify_all();
        }
    }
}

inline void ft_thread_pool::set_error(int error) const
{
    this->_error_code.store(error, std::memory_order_relaxed);
    ft_errno = error;
}

inline ft_thread_pool::ft_thread_pool(size_t thread_count, size_t max_tasks)
    : _workers(), _tasks(), _max_tasks(max_tasks), _stop(false), _active(0), _error_code(ER_SUCCESS)
{
    try
    {
        this->_workers.reserve(thread_count);
        size_t worker_index = 0;
        while (worker_index < thread_count)
        {
            this->_workers.emplace_back(&ft_thread_pool::worker, this);
            ++worker_index;
        }
    }
    catch (...)
    {
        this->set_error(THREAD_POOL_ALLOC_FAIL);
        this->_stop = true;
    }
}

inline ft_thread_pool::~ft_thread_pool()
{
    this->destroy();
}

template <typename Function>
inline void ft_thread_pool::submit(Function &&function)
{
    std::unique_lock<std::mutex> lock(this->_mutex);
    if (this->_stop)
        return ;
    if (this->_max_tasks != 0 && this->_tasks.size() >= this->_max_tasks)
    {
        this->set_error(THREAD_POOL_FULL);
        return ;
    }
    this->_tasks.emplace(std::forward<Function>(function));
    lock.unlock();
    this->_cond.notify_one();
}

inline void ft_thread_pool::wait()
{
    std::unique_lock<std::mutex> lock(this->_mutex);
      this->_cond.wait(lock, [this]() { return (this->_tasks.empty() && this->_active == 0); });
}

inline void ft_thread_pool::destroy()
{
    {
        std::unique_lock<std::mutex> lock(this->_mutex);
        this->_stop = true;
    }
    this->_cond.notify_all();
    size_t worker_index = 0;
    while (worker_index < this->_workers.size())
    {
        if (this->_workers[worker_index].joinable())
            this->_workers[worker_index].join();
        ++worker_index;
    }
    this->_workers.clear();
}

inline int ft_thread_pool::get_error() const
{
      return (this->_error_code.load(std::memory_order_relaxed));
}

inline const char* ft_thread_pool::get_error_str() const
{
      return (ft_strerror(this->_error_code.load(std::memory_order_relaxed)));
}

#endif // FT_THREAD_POOL_HPP
