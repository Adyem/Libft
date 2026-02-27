#ifndef FT_GRAPH_HPP
#define FT_GRAPH_HPP

#include "constructor.hpp"
#include "move.hpp"
#include "vector.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstddef>
#include <cstdint>
#include <new>

template <typename VertexType>
class ft_graph
{
    private:
        struct graph_node
        {
            VertexType *_value_pointer;
            size_t     *_edges;
            size_t      _degree;
            size_t      _edge_capacity;
        };

        graph_node                *_nodes;
        size_t                     _node_capacity;
        size_t                     _size;
        size_t                     _initial_capacity;
        mutable pt_recursive_mutex *_mutex;
        uint8_t                    _initialized_state;

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
            pf_printf_fd(2, "ft_graph lifecycle error: %s: %s\n", method_name, reason);
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
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
            if (lock_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_result));
            if (lock_acquired != ft_nullptr)
                *lock_acquired = true;
            return (FT_ERR_SUCCESS);
        }

        int unlock_internal(bool lock_acquired) const
        {
            int unlock_result;

            if (lock_acquired == false)
                return (FT_ERR_SUCCESS);
            if (this->_mutex == ft_nullptr)
                return (FT_ERR_SUCCESS);
            unlock_result = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
            if (unlock_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_result));
            return (FT_ERR_SUCCESS);
        }

        void reset_node(graph_node &node)
        {
            node._value_pointer = ft_nullptr;
            node._edges = ft_nullptr;
            node._degree = 0;
            node._edge_capacity = 0;
            return ;
        }

        void destroy_node(graph_node &node)
        {
            if (node._value_pointer != ft_nullptr)
            {
                destroy_at(node._value_pointer);
                cma_free(node._value_pointer);
            }
            if (node._edges != ft_nullptr)
                cma_free(node._edges);
            this->reset_node(node);
            return ;
        }

        bool ensure_node_capacity(size_t desired_capacity)
        {
            size_t new_capacity;
            graph_node *new_nodes;
            size_t index;

            if (desired_capacity <= this->_node_capacity)
                return (true);
            if (this->_node_capacity == 0)
                new_capacity = 1;
            else
                new_capacity = this->_node_capacity * 2;
            while (new_capacity < desired_capacity)
                new_capacity *= 2;
            new_nodes = static_cast<graph_node*>(cma_malloc(sizeof(graph_node) * new_capacity));
            if (new_nodes == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_NO_MEMORY);
                return (false);
            }
            index = 0;
            while (index < new_capacity)
            {
                new_nodes[index]._value_pointer = ft_nullptr;
                new_nodes[index]._edges = ft_nullptr;
                new_nodes[index]._degree = 0;
                new_nodes[index]._edge_capacity = 0;
                ++index;
            }
            index = 0;
            while (index < this->_size)
            {
                new_nodes[index]._value_pointer = this->_nodes[index]._value_pointer;
                new_nodes[index]._edges = this->_nodes[index]._edges;
                new_nodes[index]._degree = this->_nodes[index]._degree;
                new_nodes[index]._edge_capacity = this->_nodes[index]._edge_capacity;
                this->reset_node(this->_nodes[index]);
                ++index;
            }
            if (this->_nodes != ft_nullptr)
                cma_free(this->_nodes);
            this->_nodes = new_nodes;
            this->_node_capacity = new_capacity;
            set_last_operation_error(FT_ERR_SUCCESS);
            return (true);
        }

        bool ensure_edge_capacity(graph_node &node, size_t desired_capacity)
        {
            size_t new_capacity;
            size_t *new_edges;
            size_t index;

            if (desired_capacity <= node._edge_capacity)
                return (true);
            if (node._edge_capacity == 0)
                new_capacity = 1;
            else
                new_capacity = node._edge_capacity * 2;
            while (new_capacity < desired_capacity)
                new_capacity *= 2;
            new_edges = static_cast<size_t*>(cma_malloc(sizeof(size_t) * new_capacity));
            if (new_edges == ft_nullptr)
            {
                set_last_operation_error(FT_ERR_NO_MEMORY);
                return (false);
            }
            index = 0;
            while (index < node._degree)
            {
                new_edges[index] = node._edges[index];
                ++index;
            }
            if (node._edges != ft_nullptr)
                cma_free(node._edges);
            node._edges = new_edges;
            node._edge_capacity = new_capacity;
            set_last_operation_error(FT_ERR_SUCCESS);
            return (true);
        }

        void destroy_all_nodes_unlocked()
        {
            size_t index;

            index = 0;
            while (index < this->_size)
            {
                this->destroy_node(this->_nodes[index]);
                ++index;
            }
            this->_size = 0;
            return ;
        }

    public:
        explicit ft_graph(size_t initial_capacity = 0)
            : _nodes(ft_nullptr), _node_capacity(0), _size(0),
              _initial_capacity(initial_capacity), _mutex(ft_nullptr),
              _initialized_state(_state_uninitialized)
        {
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        ~ft_graph()
        {
            if (this->_initialized_state == _state_initialized)
                (void)this->destroy();
            if (this->_mutex != ft_nullptr)
                (void)this->disable_thread_safety();
            return ;
        }

        ft_graph(const ft_graph&) = delete;
        ft_graph& operator=(const ft_graph&) = delete;
        ft_graph(ft_graph&& other) = delete;
        ft_graph& operator=(ft_graph&& other) = delete;

        int initialize()
        {
            size_t index;

            if (this->_initialized_state == _state_initialized)
            {
                this->abort_lifecycle_error("ft_graph::initialize",
                    "called while object is already initialized");
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            }
            this->_nodes = ft_nullptr;
            this->_node_capacity = 0;
            this->_size = 0;
            if (this->_initial_capacity > 0)
            {
                this->_nodes = static_cast<graph_node*>(cma_malloc(sizeof(graph_node)
                        * this->_initial_capacity));
                if (this->_nodes == ft_nullptr)
                {
                    this->_initialized_state = _state_destroyed;
                    return (set_last_operation_error(FT_ERR_NO_MEMORY));
                }
                this->_node_capacity = this->_initial_capacity;
                index = 0;
                while (index < this->_node_capacity)
                {
                    this->_nodes[index]._value_pointer = ft_nullptr;
                    this->_nodes[index]._edges = ft_nullptr;
                    this->_nodes[index]._degree = 0;
                    this->_nodes[index]._edge_capacity = 0;
                    ++index;
                }
            }
            this->_initialized_state = _state_initialized;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int destroy()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            if (this->_initialized_state != _state_initialized)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(lock_error));
            this->destroy_all_nodes_unlocked();
            if (this->_nodes != ft_nullptr)
            {
                cma_free(this->_nodes);
                this->_nodes = ft_nullptr;
            }
            this->_node_capacity = 0;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (set_last_operation_error(unlock_error));
            this->_initialized_state = _state_destroyed;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        size_t add_vertex(const VertexType& value)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            VertexType *new_value;
            size_t index;

            this->abort_if_not_initialized("ft_graph::add_vertex(copy)");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (this->_size);
            if (this->ensure_node_capacity(this->_size + 1) == false)
            {
                (void)this->unlock_internal(lock_acquired);
                return (this->_size);
            }
            new_value = static_cast<VertexType*>(cma_malloc(sizeof(VertexType)));
            if (new_value == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
                set_last_operation_error(FT_ERR_NO_MEMORY);
                return (this->_size);
            }
            construct_at(new_value, value);
            index = this->_size;
            this->_nodes[index]._value_pointer = new_value;
            this->_nodes[index]._edges = ft_nullptr;
            this->_nodes[index]._degree = 0;
            this->_nodes[index]._edge_capacity = 0;
            this->_size += 1;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (this->_size);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (index);
        }

        size_t add_vertex(VertexType&& value)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            VertexType *new_value;
            size_t index;

            this->abort_if_not_initialized("ft_graph::add_vertex(move)");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (this->_size);
            if (this->ensure_node_capacity(this->_size + 1) == false)
            {
                (void)this->unlock_internal(lock_acquired);
                return (this->_size);
            }
            new_value = static_cast<VertexType*>(cma_malloc(sizeof(VertexType)));
            if (new_value == ft_nullptr)
            {
                (void)this->unlock_internal(lock_acquired);
                set_last_operation_error(FT_ERR_NO_MEMORY);
                return (this->_size);
            }
            construct_at(new_value, ft_move(value));
            index = this->_size;
            this->_nodes[index]._value_pointer = new_value;
            this->_nodes[index]._edges = ft_nullptr;
            this->_nodes[index]._degree = 0;
            this->_nodes[index]._edge_capacity = 0;
            this->_size += 1;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (this->_size);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (index);
        }

        void add_edge(size_t from, size_t to)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            graph_node *node;

            this->abort_if_not_initialized("ft_graph::add_edge");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            if (from >= this->_size || to >= this->_size)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_NOT_FOUND);
                return ;
            }
            node = &this->_nodes[from];
            if (this->ensure_edge_capacity(*node, node->_degree + 1) == false)
            {
                (void)this->unlock_internal(lock_acquired);
                return ;
            }
            node->_edges[node->_degree] = to;
            node->_degree += 1;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        template <typename Func>
        void bfs(size_t start, Func visit)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            bool *visited;
            size_t *queue_buffer;
            size_t queue_head;
            size_t queue_tail;
            size_t current;
            size_t edge_index;
            size_t adjacent;

            this->abort_if_not_initialized("ft_graph::bfs");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            if (start >= this->_size)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_NOT_FOUND);
                return ;
            }
            visited = static_cast<bool*>(cma_malloc(sizeof(bool) * this->_size));
            queue_buffer = static_cast<size_t*>(cma_malloc(sizeof(size_t) * this->_size));
            if (visited == ft_nullptr || queue_buffer == ft_nullptr)
            {
                if (visited != ft_nullptr)
                    cma_free(visited);
                if (queue_buffer != ft_nullptr)
                    cma_free(queue_buffer);
                (void)this->unlock_internal(lock_acquired);
                set_last_operation_error(FT_ERR_NO_MEMORY);
                return ;
            }
            current = 0;
            while (current < this->_size)
            {
                visited[current] = false;
                ++current;
            }
            queue_head = 0;
            queue_tail = 0;
            visited[start] = true;
            queue_buffer[queue_tail] = start;
            queue_tail += 1;
            while (queue_head < queue_tail)
            {
                current = queue_buffer[queue_head];
                queue_head += 1;
                visit(*this->_nodes[current]._value_pointer);
                edge_index = 0;
                while (edge_index < this->_nodes[current]._degree)
                {
                    adjacent = this->_nodes[current]._edges[edge_index];
                    if (visited[adjacent] == false)
                    {
                        visited[adjacent] = true;
                        queue_buffer[queue_tail] = adjacent;
                        queue_tail += 1;
                    }
                    ++edge_index;
                }
            }
            cma_free(queue_buffer);
            cma_free(visited);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        template <typename Func>
        void dfs(size_t start, Func visit)
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            bool *visited;
            size_t *stack_buffer;
            size_t stack_size;
            size_t current;
            size_t edge_index;
            size_t adjacent;

            this->abort_if_not_initialized("ft_graph::dfs");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            if (start >= this->_size)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_NOT_FOUND);
                return ;
            }
            visited = static_cast<bool*>(cma_malloc(sizeof(bool) * this->_size));
            stack_buffer = static_cast<size_t*>(cma_malloc(sizeof(size_t) * this->_size));
            if (visited == ft_nullptr || stack_buffer == ft_nullptr)
            {
                if (visited != ft_nullptr)
                    cma_free(visited);
                if (stack_buffer != ft_nullptr)
                    cma_free(stack_buffer);
                (void)this->unlock_internal(lock_acquired);
                set_last_operation_error(FT_ERR_NO_MEMORY);
                return ;
            }
            current = 0;
            while (current < this->_size)
            {
                visited[current] = false;
                ++current;
            }
            stack_size = 0;
            stack_buffer[stack_size] = start;
            stack_size += 1;
            while (stack_size > 0)
            {
                stack_size -= 1;
                current = stack_buffer[stack_size];
                if (visited[current])
                    continue ;
                visited[current] = true;
                visit(*this->_nodes[current]._value_pointer);
                edge_index = this->_nodes[current]._degree;
                while (edge_index > 0)
                {
                    edge_index -= 1;
                    adjacent = this->_nodes[current]._edges[edge_index];
                    if (visited[adjacent] == false)
                    {
                        stack_buffer[stack_size] = adjacent;
                        stack_size += 1;
                    }
                }
            }
            cma_free(stack_buffer);
            cma_free(visited);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        void neighbors(size_t index, ft_vector<size_t> &out) const
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;
            size_t edge_index;

            this->abort_if_not_initialized("ft_graph::neighbors");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            if (index >= this->_size)
            {
                unlock_error = this->unlock_internal(lock_acquired);
                if (unlock_error != FT_ERR_SUCCESS)
                    set_last_operation_error(unlock_error);
                else
                    set_last_operation_error(FT_ERR_NOT_FOUND);
                return ;
            }
            edge_index = 0;
            while (edge_index < this->_nodes[index]._degree)
            {
                out.push_back(this->_nodes[index]._edges[edge_index]);
                ++edge_index;
            }
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        size_t size() const
        {
            size_t value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_graph::size");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (0);
            value = this->_size;
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (0);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (value);
        }

        bool empty() const
        {
            bool value;
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_graph::empty");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
                return (true);
            value = (this->_size == 0);
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
                return (true);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (value);
        }

        void clear()
        {
            bool lock_acquired;
            int lock_error;
            int unlock_error;

            this->abort_if_not_initialized("ft_graph::clear");
            lock_acquired = false;
            lock_error = this->lock_internal(&lock_acquired);
            if (lock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(lock_error);
                return ;
            }
            this->destroy_all_nodes_unlocked();
            unlock_error = this->unlock_internal(lock_acquired);
            if (unlock_error != FT_ERR_SUCCESS)
            {
                set_last_operation_error(unlock_error);
                return ;
            }
            set_last_operation_error(FT_ERR_SUCCESS);
            return ;
        }

        int enable_thread_safety()
        {
            pt_recursive_mutex *new_mutex;
            int initialize_result;

            this->abort_if_not_initialized("ft_graph::enable_thread_safety");
            if (this->_mutex != ft_nullptr)
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
            this->_mutex = new_mutex;
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        int disable_thread_safety()
        {
            pt_recursive_mutex *mutex_pointer;
            int destroy_result;

            if (this->_initialized_state != _state_initialized
                && this->_initialized_state != _state_destroyed)
                return (set_last_operation_error(FT_ERR_INVALID_STATE));
            mutex_pointer = this->_mutex;
            if (mutex_pointer == ft_nullptr)
                return (set_last_operation_error(FT_ERR_SUCCESS));
            this->_mutex = ft_nullptr;
            destroy_result = mutex_pointer->destroy();
            delete mutex_pointer;
            if (destroy_result != FT_ERR_SUCCESS)
                return (set_last_operation_error(destroy_result));
            return (set_last_operation_error(FT_ERR_SUCCESS));
        }

        bool is_thread_safe() const
        {
            this->abort_if_not_initialized("ft_graph::is_thread_safe");
            set_last_operation_error(FT_ERR_SUCCESS);
            return (this->_mutex != ft_nullptr);
        }

        int lock(bool *lock_acquired) const
        {
            int lock_result;

            this->abort_if_not_initialized("ft_graph::lock");
            lock_result = this->lock_internal(lock_acquired);
            if (lock_result != FT_ERR_SUCCESS)
                return (-1);
            set_last_operation_error(FT_ERR_SUCCESS);
            return (0);
        }

        void unlock(bool lock_acquired) const
        {
            (void)this->unlock_internal(lock_acquired);
            return ;
        }

        static int32_t last_operation_error() noexcept
        {
            return (_last_error);
        }

        static const char *last_operation_error_str() noexcept
        {
            return (ft_strerror(_last_error));
        }

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *mutex_handle() const noexcept
        {
            return (this->_mutex);
        }
#endif
};

template <typename VertexType>
thread_local int32_t ft_graph<VertexType>::_last_error = FT_ERR_SUCCESS;

#endif
