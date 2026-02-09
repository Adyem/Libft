#ifndef FT_GRAPH_HPP
#define FT_GRAPH_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "queue.hpp"
#include "stack.hpp"
#include <cstddef>
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
        mutable pt_recursive_mutex *_mutex;

        int     prepare_thread_safety();
        bool    ensure_node_capacity(size_t desired);
        bool    ensure_edge_capacity(GraphNode& node, size_t desired);
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
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
        void   clear();

        int    enable_thread_safety();
        void   disable_thread_safety();
        bool   is_thread_safe() const;
        int    lock(bool *lock_acquired) const;
        void   unlock(bool lock_acquired) const;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *mutex_handle() const noexcept;
#endif
};

template <typename VertexType>
ft_graph<VertexType>::ft_graph(size_t initialCapacity)
    : _nodes(ft_nullptr), _capacity(0), _size(0),
      _mutex(ft_nullptr)
{
    if (initialCapacity > 0)
    {
        this->_nodes = static_cast<GraphNode*>(cma_malloc(sizeof(GraphNode) * initialCapacity));
        if (this->_nodes == ft_nullptr)
        {
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return ;
        }
        this->_capacity = initialCapacity;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
    : _nodes(ft_nullptr), _capacity(0), _size(0),
      _mutex(ft_nullptr)
{
    bool   other_lock_acquired;
    bool   other_thread_safe;

    other_lock_acquired = false;
    other_thread_safe = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        return ;
    }
    other_thread_safe = (other._mutex != ft_nullptr);
    this->_nodes = other._nodes;
    this->_capacity = other._capacity;
    this->_size = other._size;
    other._nodes = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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

        this_lock_acquired = false;
        other_lock_acquired = false;
        other_thread_safe = false;
        if (this->lock_internal(&this_lock_acquired) != 0)
            return (*this);
        this->clear();
        if (this->_nodes != ft_nullptr)
            cma_free(this->_nodes);
        this->_nodes = ft_nullptr;
        this->_capacity = 0;
        this->_size = 0;
        this->unlock_internal(this_lock_acquired);
        this->teardown_thread_safety();
        if (other.lock_internal(&other_lock_acquired) != 0)
            return (*this);
        other_thread_safe = (other._mutex != ft_nullptr);
        this->_nodes = other._nodes;
        this->_capacity = other._capacity;
        this->_size = other._size;
        other._nodes = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other.unlock_internal(other_lock_acquired);
        other.teardown_thread_safety();
        if (other_thread_safe)
        {
            if (this->enable_thread_safety() != 0)
                return (*this);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename VertexType>
bool ft_graph<VertexType>::ensure_node_capacity(size_t desired)
{
    if (desired <= this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

template <typename VertexType>
bool ft_graph<VertexType>::ensure_edge_capacity(GraphNode& node, size_t desired)
{
    if (desired <= node._capacity)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

template <typename VertexType>
size_t ft_graph<VertexType>::add_vertex(const VertexType& value)
{
    bool   lock_acquired;
    size_t original_size;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (this->_size);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        return (this->_size);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (idx);
}

template <typename VertexType>
void ft_graph<VertexType>::add_edge(size_t from, size_t to)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    if (from >= this->_size || to >= this->_size)
    {
        ft_global_error_stack_push(FT_ERR_NOT_FOUND);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        return ;
    if (start >= this->_size)
    {
        ft_global_error_stack_push(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return ;
    }
    bool* visited = static_cast<bool*>(cma_malloc(sizeof(bool) * this->_size));
    if (visited == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        return ;
    if (start >= this->_size)
    {
        ft_global_error_stack_push(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return ;
    }
    bool* visited = static_cast<bool*>(cma_malloc(sizeof(bool) * this->_size));
    if (visited == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename VertexType>
void ft_graph<VertexType>::neighbors(size_t index, ft_vector<size_t> &out) const
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    if (index >= this->_size)
    {
        ft_global_error_stack_push(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return ;
    }
    size_t neighbor_index = 0;
    while (neighbor_index < this->_nodes[index]._degree)
    {
        out.push_back(this->_nodes[index]._edges[neighbor_index]);
        ++neighbor_index;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        return (this->_size);
    size_value = this->_size;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        return (true);
    result = (this->_size == 0);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename VertexType>
void ft_graph<VertexType>::clear()
{
    bool   lock_acquired;
    size_t node_index_clear = 0;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename VertexType>
int ft_graph<VertexType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

template <typename VertexType>
int ft_graph<VertexType>::enable_thread_safety()
{
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename VertexType>
void ft_graph<VertexType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename VertexType>
bool ft_graph<VertexType>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename VertexType>
int ft_graph<VertexType>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);
    if (result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(result);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

template <typename VertexType>
void ft_graph<VertexType>::unlock(bool lock_acquired) const
{
    int unlock_error = this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(unlock_error);
    return ;
}

template <typename VertexType>
int ft_graph<VertexType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int result = pt_recursive_mutex_lock_with_error(*this->_mutex);
    if (result == FT_ERR_SUCCESSS && lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (result);
}

template <typename VertexType>
int ft_graph<VertexType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename VertexType>
void ft_graph<VertexType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

#ifdef LIBFT_TEST_BUILD
template <typename VertexType>
pt_recursive_mutex *ft_graph<VertexType>::mutex_handle() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
