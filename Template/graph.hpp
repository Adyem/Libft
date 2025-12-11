#ifndef FT_GRAPH_HPP
#define FT_GRAPH_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "queue.hpp"
#include "stack.hpp"
#include <cstddef>
#include <utility>
#include "vector.hpp"

#include "move.hpp"
template <typename VertexType>
class ft_graph
{
    private:
        struct GraphNode
        {
            VertexType  _value;
            size_t*     _edges;
            size_t      _degree;
            size_t      _capacity;
        };

        GraphNode*       _nodes;
        size_t           _capacity;
        size_t           _size;
        mutable int      _error_code;
        mutable pt_mutex *_mutex;
        bool             _thread_safe_enabled;

        void    set_error(int error) const;
        bool    ensure_node_capacity(size_t desired);
        bool    ensure_edge_capacity(GraphNode& node, size_t desired);
        int     lock_internal(bool *lock_acquired) const;
        void    unlock_internal(bool lock_acquired) const;
        void    teardown_thread_safety();

    public:
        ft_graph(size_t initialCapacity = 0);
        ~ft_graph();

        ft_graph(const ft_graph&) = delete;
        ft_graph& operator=(const ft_graph&) = delete;

        ft_graph(ft_graph&& other) noexcept;
        ft_graph& operator=(ft_graph&& other) noexcept;

        size_t add_vertex(const VertexType& value);
        size_t add_vertex(VertexType&& value);
        void   add_edge(size_t from, size_t to);

        template <typename Func>
        void bfs(size_t start, Func visit);

        template <typename Func>
        void dfs(size_t start, Func visit);

        void neighbors(size_t index, ft_vector<size_t> &out) const;

        size_t size() const;
        bool   empty() const;
        int    get_error() const;
        const char* get_error_str() const;
        void   clear();

        int    enable_thread_safety();
        void   disable_thread_safety();
        bool   is_thread_safe() const;
        int    lock(bool *lock_acquired) const;
        void   unlock(bool lock_acquired) const;
};

template <typename VertexType>
ft_graph<VertexType>::ft_graph(size_t initialCapacity)
    : _nodes(ft_nullptr), _capacity(0), _size(0), _error_code(FT_ERR_SUCCESSS),
      _mutex(ft_nullptr), _thread_safe_enabled(false)
{
    if (initialCapacity > 0)
    {
        this->_nodes = static_cast<GraphNode*>(cma_malloc(sizeof(GraphNode) * initialCapacity));
        if (this->_nodes == ft_nullptr)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return ;
        }
        this->_capacity = initialCapacity;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename VertexType>
ft_graph<VertexType>::~ft_graph()
{
    if (this->_nodes != ft_nullptr)
    {
        size_t node_index = 0;
        while (node_index < this->_size)
        {
            destroy_at(&this->_nodes[node_index]._value);
            if (this->_nodes[node_index]._edges != ft_nullptr)
                cma_free(this->_nodes[node_index]._edges);
            ++node_index;
        }
        cma_free(this->_nodes);
    }
    this->teardown_thread_safety();
    return ;
}

template <typename VertexType>
ft_graph<VertexType>::ft_graph(ft_graph&& other) noexcept
    : _nodes(ft_nullptr), _capacity(0), _size(0), _error_code(FT_ERR_SUCCESSS),
      _mutex(ft_nullptr), _thread_safe_enabled(false)
{
    bool   other_lock_acquired;
    bool   other_thread_safe;
    int    other_error_code;

    other_lock_acquired = false;
    other_thread_safe = false;
    other_error_code = FT_ERR_SUCCESSS;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    other_thread_safe = (other._thread_safe_enabled && other._mutex != ft_nullptr);
    other_error_code = other._error_code;
    this->_nodes = other._nodes;
    this->_capacity = other._capacity;
    this->_size = other._size;
    other._nodes = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._error_code = FT_ERR_SUCCESSS;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->_error_code = other_error_code;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename VertexType>
ft_graph<VertexType>& ft_graph<VertexType>::operator=(ft_graph&& other) noexcept
{
    if (this != &other)
    {
        bool   this_lock_acquired;
        bool   other_lock_acquired;
        bool   other_thread_safe;
        int    other_error_code;

        this_lock_acquired = false;
        other_lock_acquired = false;
        other_thread_safe = false;
        other_error_code = FT_ERR_SUCCESSS;
        if (this->lock_internal(&this_lock_acquired) != 0)
        {
            this->set_error(ft_errno);
            return (*this);
        }
        this->clear();
        if (this->_nodes != ft_nullptr)
            cma_free(this->_nodes);
        this->_nodes = ft_nullptr;
        this->_capacity = 0;
        this->_size = 0;
        this->_error_code = FT_ERR_SUCCESSS;
        this->unlock_internal(this_lock_acquired);
        this->teardown_thread_safety();
        if (other.lock_internal(&other_lock_acquired) != 0)
        {
            this->set_error(ft_errno);
            return (*this);
        }
        other_thread_safe = (other._thread_safe_enabled && other._mutex != ft_nullptr);
        other_error_code = other._error_code;
        this->_nodes = other._nodes;
        this->_capacity = other._capacity;
        this->_size = other._size;
        other._nodes = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other._error_code = FT_ERR_SUCCESSS;
        other.unlock_internal(other_lock_acquired);
        other.teardown_thread_safety();
        if (other_thread_safe)
        {
            if (this->enable_thread_safety() != 0)
                return (*this);
        }
        this->_error_code = other_error_code;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename VertexType>
void ft_graph<VertexType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename VertexType>
bool ft_graph<VertexType>::ensure_node_capacity(size_t desired)
{
    if (desired <= this->_capacity)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (true);
    }
    size_t newCap;
    if (this->_capacity == 0)
        newCap = 1;
    else
        newCap = this->_capacity * 2;
    while (newCap < desired)
        newCap *= 2;
    GraphNode* newNodes = static_cast<GraphNode*>(cma_malloc(sizeof(GraphNode) * newCap));
    if (newNodes == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (false);
    }
    size_t node_index = 0;
    while (node_index < this->_size)
    {
        construct_at(&newNodes[node_index]._value, ft_move(this->_nodes[node_index]._value));
        newNodes[node_index]._edges = this->_nodes[node_index]._edges;
        newNodes[node_index]._degree = this->_nodes[node_index]._degree;
        newNodes[node_index]._capacity = this->_nodes[node_index]._capacity;
        destroy_at(&this->_nodes[node_index]._value);
        ++node_index;
    }
    if (this->_nodes != ft_nullptr)
        cma_free(this->_nodes);
    this->_nodes = newNodes;
    this->_capacity = newCap;
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

template <typename VertexType>
bool ft_graph<VertexType>::ensure_edge_capacity(GraphNode& node, size_t desired)
{
    if (desired <= node._capacity)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (true);
    }
    size_t newCap;
    if (node._capacity == 0)
        newCap = 1;
    else
        newCap = node._capacity * 2;
    while (newCap < desired)
        newCap *= 2;
    size_t* newEdges = static_cast<size_t*>(cma_malloc(sizeof(size_t) * newCap));
    if (newEdges == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (false);
    }
    size_t edge_index = 0;
    while (edge_index < node._degree)
    {
        newEdges[edge_index] = node._edges[edge_index];
        ++edge_index;
    }
    if (node._edges != ft_nullptr)
        cma_free(node._edges);
    node._edges = newEdges;
    node._capacity = newCap;
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

template <typename VertexType>
size_t ft_graph<VertexType>::add_vertex(const VertexType& value)
{
    bool   lock_acquired;
    size_t original_size;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (this->_size);
    }
    original_size = this->_size;
    if (!ensure_node_capacity(this->_size + 1))
    {
        this->unlock_internal(lock_acquired);
        return (original_size);
    }
    construct_at(&this->_nodes[this->_size]._value, value);
    this->_nodes[this->_size]._edges = ft_nullptr;
    this->_nodes[this->_size]._degree = 0;
    this->_nodes[this->_size]._capacity = 0;
    size_t idx = this->_size;
    ++this->_size;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (idx);
}

template <typename VertexType>
size_t ft_graph<VertexType>::add_vertex(VertexType&& value)
{
    bool   lock_acquired;
    size_t original_size;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (this->_size);
    }
    original_size = this->_size;
    if (!ensure_node_capacity(this->_size + 1))
    {
        this->unlock_internal(lock_acquired);
        return (original_size);
    }
    construct_at(&this->_nodes[this->_size]._value, ft_move(value));
    this->_nodes[this->_size]._edges = ft_nullptr;
    this->_nodes[this->_size]._degree = 0;
    this->_nodes[this->_size]._capacity = 0;
    size_t idx = this->_size;
    ++this->_size;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (idx);
}

template <typename VertexType>
void ft_graph<VertexType>::add_edge(size_t from, size_t to)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (from >= this->_size || to >= this->_size)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return ;
    }
    GraphNode& node = this->_nodes[from];
    if (!ensure_edge_capacity(node, node._degree + 1))
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    node._edges[node._degree] = to;
    ++node._degree;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename VertexType>
template <typename Func>
void ft_graph<VertexType>::bfs(size_t start, Func visit)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (start >= this->_size)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return ;
    }
    bool* visited = static_cast<bool*>(cma_malloc(sizeof(bool) * this->_size));
    if (visited == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    size_t node_index = 0;
    while (node_index < this->_size)
    {
        visited[node_index] = false;
        ++node_index;
    }
    ft_queue<size_t> q;
    q.enqueue(start);
    visited[start] = true;
    while (!q.empty())
    {
        size_t v = q.dequeue();
        visit(this->_nodes[v]._value);
        size_t neighbor_index = 0;
        while (neighbor_index < this->_nodes[v]._degree)
        {
            size_t w = this->_nodes[v]._edges[neighbor_index];
            if (!visited[w])
            {
                visited[w] = true;
                q.enqueue(w);
            }
            ++neighbor_index;
        }
    }
    cma_free(visited);
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename VertexType>
template <typename Func>
void ft_graph<VertexType>::dfs(size_t start, Func visit)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (start >= this->_size)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return ;
    }
    bool* visited = static_cast<bool*>(cma_malloc(sizeof(bool) * this->_size));
    if (visited == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    size_t node_index = 0;
    while (node_index < this->_size)
    {
        visited[node_index] = false;
        ++node_index;
    }
    ft_stack<size_t> st;
    st.push(start);
    while (!st.empty())
    {
        size_t v = st.pop();
        if (visited[v])
            continue ;
        visited[v] = true;
        visit(this->_nodes[v]._value);
        size_t neighbor_index = this->_nodes[v]._degree;
        while (neighbor_index > 0)
        {
            --neighbor_index;
            size_t w = this->_nodes[v]._edges[neighbor_index];
            if (!visited[w])
                st.push(w);
        }
    }
    cma_free(visited);
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename VertexType>
void ft_graph<VertexType>::neighbors(size_t index, ft_vector<size_t> &out) const
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_graph<VertexType> *>(this)->set_error(ft_errno);
        return ;
    }
    if (index >= this->_size)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return ;
    }
    size_t neighbor_index = 0;
    while (neighbor_index < this->_nodes[index]._degree)
    {
        out.push_back(this->_nodes[index]._edges[neighbor_index]);
        ++neighbor_index;
    }
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename VertexType>
size_t ft_graph<VertexType>::size() const
{
    size_t size_value;
    bool   lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_graph<VertexType> *>(this)->set_error(ft_errno);
        return (this->_size);
    }
    size_value = this->_size;
    const_cast<ft_graph<VertexType> *>(this)->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (size_value);
}

template <typename VertexType>
bool ft_graph<VertexType>::empty() const
{
    bool result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_graph<VertexType> *>(this)->set_error(ft_errno);
        return (true);
    }
    result = (this->_size == 0);
    const_cast<ft_graph<VertexType> *>(this)->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename VertexType>
int ft_graph<VertexType>::get_error() const
{
    int err = this->_error_code;
    this->set_error(err);
    return (err);
}

template <typename VertexType>
const char* ft_graph<VertexType>::get_error_str() const
{
    int err = this->_error_code;
    this->set_error(err);
    return (ft_strerror(err));
}

template <typename VertexType>
void ft_graph<VertexType>::clear()
{
    bool   lock_acquired;
    size_t node_index_clear = 0;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    while (node_index_clear < this->_size)
    {
        destroy_at(&this->_nodes[node_index_clear]._value);
        if (this->_nodes[node_index_clear]._edges != ft_nullptr)
        {
            cma_free(this->_nodes[node_index_clear]._edges);
            this->_nodes[node_index_clear]._edges = ft_nullptr;
            this->_nodes[node_index_clear]._degree = 0;
            this->_nodes[node_index_clear]._capacity = 0;
        }
        ++node_index_clear;
    }
    this->_size = 0;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename VertexType>
int ft_graph<VertexType>::enable_thread_safety()
{
    void     *memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

template <typename VertexType>
void ft_graph<VertexType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename VertexType>
bool ft_graph<VertexType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<ft_graph<VertexType> *>(this)->set_error(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename VertexType>
int ft_graph<VertexType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_graph<VertexType> *>(this)->set_error(ft_errno);
    else
        const_cast<ft_graph<VertexType> *>(this)->set_error(FT_ERR_SUCCESSS);
    return (result);
}

template <typename VertexType>
void ft_graph<VertexType>::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_mutex != ft_nullptr && this->_mutex->get_error() != FT_ERR_SUCCESSS)
        const_cast<ft_graph<VertexType> *>(this)->set_error(this->_mutex->get_error());
    else
    {
        ft_errno = entry_errno;
        const_cast<ft_graph<VertexType> *>(this)->set_error(ft_errno);
    }
    return ;
}

template <typename VertexType>
int ft_graph<VertexType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this->_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

template <typename VertexType>
void ft_graph<VertexType>::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this->_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template <typename VertexType>
void ft_graph<VertexType>::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_mutex();
        cma_free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

#endif
