#ifndef FT_GRAPH_HPP
#define FT_GRAPH_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "queue.hpp"
#include "stack.hpp"
#include <cstddef>
#include <utility>
#include "move.hpp"
#include "vector.hpp"



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

        GraphNode*   _nodes;
        size_t       _capacity;
        size_t       _size;
        mutable int  _error_code;
        mutable pt_mutex _mutex;

        void    set_error(int error) const;
        bool    ensure_node_capacity(size_t desired);
        bool    ensure_edge_capacity(GraphNode& node, size_t desired);

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
};

template <typename VertexType>
ft_graph<VertexType>::ft_graph(size_t initialCapacity)
    : _nodes(ft_nullptr), _capacity(0), _size(0), _error_code(ER_SUCCESS)
{
    if (initialCapacity > 0)
    {
        this->_nodes = static_cast<GraphNode*>(cma_malloc(sizeof(GraphNode) * initialCapacity));
        if (this->_nodes == ft_nullptr)
            this->set_error(GRAPH_ALLOC_FAIL);
        else
            this->_capacity = initialCapacity;
    }
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
    return ;
}

template <typename VertexType>
ft_graph<VertexType>::ft_graph(ft_graph&& other) noexcept
    : _nodes(other._nodes), _capacity(other._capacity), _size(other._size), _error_code(other._error_code)
{
    other._nodes = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename VertexType>
ft_graph<VertexType>& ft_graph<VertexType>::operator=(ft_graph&& other) noexcept
{
    if (this != &other)
    {
        if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
            return (*this);
        if (other._mutex.lock(THREAD_ID) != FT_SUCCESS)
        {
            this->_mutex.unlock(THREAD_ID);
            return (*this);
        }
        this->clear();
        if (this->_nodes != ft_nullptr)
            cma_free(this->_nodes);
        this->_nodes = other._nodes;
        this->_capacity = other._capacity;
        this->_size = other._size;
        this->_error_code = other._error_code;
        other._nodes = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other._error_code = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
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
        return (true);
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
        this->set_error(GRAPH_ALLOC_FAIL);
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
    return (true);
}

template <typename VertexType>
bool ft_graph<VertexType>::ensure_edge_capacity(GraphNode& node, size_t desired)
{
    if (desired <= node._capacity)
        return (true);
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
        this->set_error(GRAPH_ALLOC_FAIL);
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
    return (true);
}

template <typename VertexType>
size_t ft_graph<VertexType>::add_vertex(const VertexType& value)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (this->_size);
    }
    if (!ensure_node_capacity(this->_size + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return (this->_size);
    }
    construct_at(&this->_nodes[this->_size]._value, value);
    this->_nodes[this->_size]._edges = ft_nullptr;
    this->_nodes[this->_size]._degree = 0;
    this->_nodes[this->_size]._capacity = 0;
    size_t idx = this->_size;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (idx);
}

template <typename VertexType>
size_t ft_graph<VertexType>::add_vertex(VertexType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (this->_size);
    }
    if (!ensure_node_capacity(this->_size + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return (this->_size);
    }
    construct_at(&this->_nodes[this->_size]._value, ft_move(value));
    this->_nodes[this->_size]._edges = ft_nullptr;
    this->_nodes[this->_size]._degree = 0;
    this->_nodes[this->_size]._capacity = 0;
    size_t idx = this->_size;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (idx);
}

template <typename VertexType>
void ft_graph<VertexType>::add_edge(size_t from, size_t to)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (from >= this->_size || to >= this->_size)
    {
        this->set_error(GRAPH_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    GraphNode& node = this->_nodes[from];
    if (!ensure_edge_capacity(node, node._degree + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    node._edges[node._degree] = to;
    ++node._degree;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename VertexType>
template <typename Func>
void ft_graph<VertexType>::bfs(size_t start, Func visit)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (start >= this->_size)
    {
        this->set_error(GRAPH_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    bool* visited = static_cast<bool*>(cma_malloc(sizeof(bool) * this->_size));
    if (visited == ft_nullptr)
    {
        this->set_error(GRAPH_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
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
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename VertexType>
template <typename Func>
void ft_graph<VertexType>::dfs(size_t start, Func visit)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (start >= this->_size)
    {
        this->set_error(GRAPH_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    bool* visited = static_cast<bool*>(cma_malloc(sizeof(bool) * this->_size));
    if (visited == ft_nullptr)
    {
        this->set_error(GRAPH_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
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
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename VertexType>
void ft_graph<VertexType>::neighbors(size_t index, ft_vector<size_t> &out) const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return ;
    if (index >= this->_size)
    {
        this->set_error(GRAPH_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    size_t neighbor_index = 0;
    while (neighbor_index < this->_nodes[index]._degree)
    {
        out.push_back(this->_nodes[index]._edges[neighbor_index]);
        ++neighbor_index;
    }
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename VertexType>
size_t ft_graph<VertexType>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (0);
    size_t s = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (s);
}

template <typename VertexType>
bool ft_graph<VertexType>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (true);
    bool res = (this->_size == 0);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename VertexType>
int ft_graph<VertexType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (this->_error_code);
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename VertexType>
const char* ft_graph<VertexType>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (ft_strerror(this->_error_code));
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

template <typename VertexType>
void ft_graph<VertexType>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return ;
    size_t node_index_clear = 0;
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
    this->_mutex.unlock(THREAD_ID);
    return ;
}

#endif
