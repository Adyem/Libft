#ifndef FT_GRAPH_HPP
#define FT_GRAPH_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "queue.hpp"
#include "stack.hpp"
#include <cstddef>
#include <utility>

/*
 * ft_graph
 * Graph data structure using adjacency lists with BFS and DFS traversal.
 */

template <typename VertexType>
class ft_graph
{
    private:
        struct GraphNode
        {
            VertexType  value;
            size_t*     edges;
            size_t      degree;
            size_t      capacity;
        };

        GraphNode*   _nodes;
        size_t       _capacity;
        size_t       _size;
        mutable int  _errorCode;
        mutable pt_mutex _mutex;

        void    setError(int error) const;
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

        size_t size() const;
        bool   empty() const;
        int    get_error() const;
        const char* get_error_str() const;
        void   clear();
};

template <typename VertexType>
ft_graph<VertexType>::ft_graph(size_t initialCapacity)
    : _nodes(ft_nullptr), _capacity(0), _size(0), _errorCode(ER_SUCCESS)
{
    if (initialCapacity > 0)
    {
        this->_nodes = static_cast<GraphNode*>(cma_malloc(sizeof(GraphNode) * initialCapacity));
        if (this->_nodes == ft_nullptr)
            this->setError(GRAPH_ALLOC_FAIL);
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
        size_t i = 0;
        while (i < this->_size)
        {
            destroy_at(&this->_nodes[i].value);
            if (this->_nodes[i].edges != ft_nullptr)
                cma_free(this->_nodes[i].edges);
            ++i;
        }
        cma_free(this->_nodes);
    }
    return ;
}

template <typename VertexType>
ft_graph<VertexType>::ft_graph(ft_graph&& other) noexcept
    : _nodes(other._nodes), _capacity(other._capacity), _size(other._size), _errorCode(other._errorCode)
{
    other._nodes = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._errorCode = ER_SUCCESS;
    return ;
}

template <typename VertexType>
ft_graph<VertexType>& ft_graph<VertexType>::operator=(ft_graph&& other) noexcept
{
    if (this != &other)
    {
        if (this->_mutex.lock(THREAD_ID) != SUCCES)
            return (*this);
        if (other._mutex.lock(THREAD_ID) != SUCCES)
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
        this->_errorCode = other._errorCode;
        other._nodes = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other._errorCode = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename VertexType>
void ft_graph<VertexType>::setError(int error) const
{
    this->_errorCode = error;
    ft_errno = error;
    return ;
}

template <typename VertexType>
bool ft_graph<VertexType>::ensure_node_capacity(size_t desired)
{
    if (desired <= this->_capacity)
        return (true);
    size_t newCap = (this->_capacity == 0) ? 1 : this->_capacity * 2;
    while (newCap < desired)
        newCap *= 2;
    GraphNode* newNodes = static_cast<GraphNode*>(cma_malloc(sizeof(GraphNode) * newCap));
    if (newNodes == ft_nullptr)
    {
        this->setError(GRAPH_ALLOC_FAIL);
        return (false);
    }
    size_t i = 0;
    while (i < this->_size)
    {
        construct_at(&newNodes[i].value, std::move(this->_nodes[i].value));
        newNodes[i].edges = this->_nodes[i].edges;
        newNodes[i].degree = this->_nodes[i].degree;
        newNodes[i].capacity = this->_nodes[i].capacity;
        destroy_at(&this->_nodes[i].value);
        ++i;
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
    if (desired <= node.capacity)
        return (true);
    size_t newCap = (node.capacity == 0) ? 1 : node.capacity * 2;
    while (newCap < desired)
        newCap *= 2;
    size_t* newEdges = static_cast<size_t*>(cma_malloc(sizeof(size_t) * newCap));
    if (newEdges == ft_nullptr)
    {
        this->setError(GRAPH_ALLOC_FAIL);
        return (false);
    }
    size_t i = 0;
    while (i < node.degree)
    {
        newEdges[i] = node.edges[i];
        ++i;
    }
    if (node.edges != ft_nullptr)
        cma_free(node.edges);
    node.edges = newEdges;
    node.capacity = newCap;
    return (true);
}

template <typename VertexType>
size_t ft_graph<VertexType>::add_vertex(const VertexType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (this->_size);
    }
    if (!ensure_node_capacity(this->_size + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return (this->_size);
    }
    construct_at(&this->_nodes[this->_size].value, value);
    this->_nodes[this->_size].edges = ft_nullptr;
    this->_nodes[this->_size].degree = 0;
    this->_nodes[this->_size].capacity = 0;
    size_t idx = this->_size;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (idx);
}

template <typename VertexType>
size_t ft_graph<VertexType>::add_vertex(VertexType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (this->_size);
    }
    if (!ensure_node_capacity(this->_size + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return (this->_size);
    }
    construct_at(&this->_nodes[this->_size].value, std::move(value));
    this->_nodes[this->_size].edges = ft_nullptr;
    this->_nodes[this->_size].degree = 0;
    this->_nodes[this->_size].capacity = 0;
    size_t idx = this->_size;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (idx);
}

template <typename VertexType>
void ft_graph<VertexType>::add_edge(size_t from, size_t to)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (from >= this->_size || to >= this->_size)
    {
        this->setError(GRAPH_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    GraphNode& node = this->_nodes[from];
    if (!ensure_edge_capacity(node, node.degree + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    node.edges[node.degree] = to;
    ++node.degree;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename VertexType>
template <typename Func>
void ft_graph<VertexType>::bfs(size_t start, Func visit)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (start >= this->_size)
    {
        this->setError(GRAPH_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    bool* visited = static_cast<bool*>(cma_malloc(sizeof(bool) * this->_size));
    if (visited == ft_nullptr)
    {
        this->setError(GRAPH_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    size_t i = 0;
    while (i < this->_size)
    {
        visited[i] = false;
        ++i;
    }
    ft_queue<size_t> q;
    q.enqueue(start);
    visited[start] = true;
    while (!q.empty())
    {
        size_t v = q.dequeue();
        visit(this->_nodes[v].value);
        size_t j = 0;
        while (j < this->_nodes[v].degree)
        {
            size_t w = this->_nodes[v].edges[j];
            if (!visited[w])
            {
                visited[w] = true;
                q.enqueue(w);
            }
            ++j;
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
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (start >= this->_size)
    {
        this->setError(GRAPH_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    bool* visited = static_cast<bool*>(cma_malloc(sizeof(bool) * this->_size));
    if (visited == ft_nullptr)
    {
        this->setError(GRAPH_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    size_t i = 0;
    while (i < this->_size)
    {
        visited[i] = false;
        ++i;
    }
    ft_stack<size_t> st;
    st.push(start);
    while (!st.empty())
    {
        size_t v = st.pop();
        if (visited[v])
            continue ;
        visited[v] = true;
        visit(this->_nodes[v].value);
        size_t j = this->_nodes[v].degree;
        while (j > 0)
        {
            --j;
            size_t w = this->_nodes[v].edges[j];
            if (!visited[w])
                st.push(w);
        }
    }
    cma_free(visited);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename VertexType>
size_t ft_graph<VertexType>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t s = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (s);
}

template <typename VertexType>
bool ft_graph<VertexType>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (true);
    bool res = (this->_size == 0);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename VertexType>
int ft_graph<VertexType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_errorCode);
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename VertexType>
const char* ft_graph<VertexType>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_strerror(this->_errorCode));
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

template <typename VertexType>
void ft_graph<VertexType>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    size_t i = 0;
    while (i < this->_size)
    {
        destroy_at(&this->_nodes[i].value);
        if (this->_nodes[i].edges != ft_nullptr)
        {
            cma_free(this->_nodes[i].edges);
            this->_nodes[i].edges = ft_nullptr;
            this->_nodes[i].degree = 0;
            this->_nodes[i].capacity = 0;
        }
        ++i;
    }
    this->_size = 0;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

#endif
