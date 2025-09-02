#ifndef FT_THREAD_POOL_HPP
#define FT_THREAD_POOL_HPP

#include "constructor.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
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
        size_t                        _maxTasks;
        bool                          _stop;
        size_t                        _active;
        mutable std::atomic<int>      _errorCode;
        std::mutex                    _mutex;
        std::condition_variable       _cond;

        void setError(int error) const;
        void worker();

    public:
        ft_thread_pool(size_t threadCount = 0, size_t maxTasks = 0);
        ~ft_thread_pool();

        ft_thread_pool(const ft_thread_pool&) = delete;
        ft_thread_pool& operator=(const ft_thread_pool&) = delete;
        ft_thread_pool(ft_thread_pool&&) = delete;
        ft_thread_pool& operator=(ft_thread_pool&&) = delete;

        template <typename Func>
        void submit(Func &&f);

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
            this->_cond.wait(lock, [this]() { return this->_stop || !this->_tasks.empty(); });
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

inline void ft_thread_pool::setError(int error) const
{
    this->_errorCode.store(error, std::memory_order_relaxed);
    ft_errno = error;
}

inline ft_thread_pool::ft_thread_pool(size_t threadCount, size_t maxTasks)
    : _workers(), _tasks(), _maxTasks(maxTasks), _stop(false), _active(0), _errorCode(ER_SUCCESS)
{
    try
    {
        this->_workers.reserve(threadCount);
        size_t i = 0;
        while (i < threadCount)
        {
            this->_workers.emplace_back(&ft_thread_pool::worker, this);
            ++i;
        }
    }
    catch (...)
    {
        this->setError(THREAD_POOL_ALLOC_FAIL);
        this->_stop = true;
    }
}

inline ft_thread_pool::~ft_thread_pool()
{
    this->destroy();
}

template <typename Func>
inline void ft_thread_pool::submit(Func &&f)
{
    std::unique_lock<std::mutex> lock(this->_mutex);
    if (this->_stop)
        return ;
    if (this->_maxTasks != 0 && this->_tasks.size() >= this->_maxTasks)
    {
        this->setError(THREAD_POOL_FULL);
        return ;
    }
    this->_tasks.emplace(std::forward<Func>(f));
    lock.unlock();
    this->_cond.notify_one();
}

inline void ft_thread_pool::wait()
{
    std::unique_lock<std::mutex> lock(this->_mutex);
    this->_cond.wait(lock, [this]() { return this->_tasks.empty() && this->_active == 0; });
}

inline void ft_thread_pool::destroy()
{
    {
        std::unique_lock<std::mutex> lock(this->_mutex);
        this->_stop = true;
    }
    this->_cond.notify_all();
    size_t i = 0;
    while (i < this->_workers.size())
    {
        if (this->_workers[i].joinable())
            this->_workers[i].join();
        ++i;
    }
    this->_workers.clear();
}

inline int ft_thread_pool::get_error() const
{
    return this->_errorCode.load(std::memory_order_relaxed);
}

inline const char* ft_thread_pool::get_error_str() const
{
    return ft_strerror(this->_errorCode.load(std::memory_order_relaxed));
}

#endif // FT_THREAD_POOL_HPP
