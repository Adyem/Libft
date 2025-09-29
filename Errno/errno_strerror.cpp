#include "errno.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include <cstddef>

typedef struct s_ft_error_string
{
    int error_code;
    const char *error_message;
}   t_ft_error_string;

static const t_ft_error_string g_error_strings[] =
{
    {ER_SUCCESS, "Operation successful"},
    {CMA_BAD_ALLOC, "Bad allocation"},
    {CMA_INVALID_PTR, "Invalid CMA pointer"},
    {PT_ERR_QUEUE_FULL, "Wait queue is full"},
    {PT_ERR_MUTEX_NULLPTR, "Mutex pointer is null"},
    {PT_ERR_MUTEX_OWNER, "Thread is not the owner of the mutex"},
    {PT_ERR_ALREADY_LOCKED, "Mutex already locked"},
    {SHARED_PTR_NULL_PTR, "Null pointer dereference"},
    {SHARED_PTR_OUT_OF_BOUNDS, "Array index out of bounds"},
    {SHARED_PTR_ALLOCATION_FAILED, "Shared pointer allocation failed"},
    {SHARED_PTR_INVALID_OPERATION, "Invalid operation on shared pointer"},
    {SHARED_PTR_ELEMENT_ALREADY_ADDED, "Element already in the array"},
    {UNIQUE_PTR_NULL_PTR, "Unique pointer null pointer dereference"},
    {UNIQUE_PTR_OUT_OF_BOUNDS, "Unique pointer index out of bounds"},
    {UNIQUE_PTR_ALLOCATION_FAILED, "Unique pointer allocation failed"},
    {UNIQUE_PTR_INVALID_OPERATION, "Invalid operation on unique pointer"},
    {MAP_ALLOCATION_FAILED, "Map memory allocation failed"},
    {MAP_KEY_NOT_FOUND, "Map key not found"},
    {FILE_INVALID_FD, "Bad file descriptor"},
    {FILE_END_OF_FILE, "End of file reached"},
    {FT_EINVAL, "Invalid argument"},
    {FT_EOVERLAP, "Memory regions overlap"},
    {FT_ERANGE, "Result out of range"},
    {STRING_MEM_ALLOC_FAIL, "String memory allocation failed"},
    {STRING_ERASE_OUT_OF_BOUNDS, "String acces out of bounds"},
    {BIG_NUMBER_ALLOC_FAIL, "Big number memory allocation failed"},
    {BIG_NUMBER_INVALID_DIGIT, "Big number digit must be numeric"},
    {BIG_NUMBER_NEGATIVE_RESULT, "Big number result would be negative"},
    {BIG_NUMBER_DIVIDE_BY_ZERO, "Big number division by zero"},
    {VECTOR_ALLOC_FAIL, "Vector memory allocation failed"},
    {VECTOR_OUT_OF_BOUNDS, "Vector index out of bounds"},
    {VECTOR_INVALID_PTR, "Vector invalid pointer"},
    {VECTOR_CRITICAL_ERROR, "Vector critical error"},
    {VECTOR_INVALID_OPERATION, "Vector invalid operation"},
    {FT_EALLOC, "Allocation error"},
    {FT_ETERM, "Terminal error"},
    {SOCKET_CREATION_FAILED, "Failed to create socket."},
    {SOCKET_BIND_FAILED, "Failed to bind socket."},
    {SOCKET_LISTEN_FAILED, "Failed to listen on socket."},
    {SOCKET_CONNECT_FAILED, "Failed to connect to server."},
    {INVALID_IP_FORMAT, "Invalid IP address format."},
    {UNSUPPORTED_SOCKET_TYPE, "Unsupported socket type."},
    {SOCKET_ACCEPT_FAILED, "Failed to accept connection."},
    {SOCKET_SEND_FAILED, "Failed to send data through socket."},
    {SOCKET_RECEIVE_FAILED, "Failed to receive data from socket."},
    {SOCKET_CLOSE_FAILED, "Failed to close socket."},
    {SOCKET_INVALID_CONFIGURATION, "Socket invalid configuration"},
    {SOCKET_UNSUPPORTED_TYPE, "Socket unsupported type"},
    {SOCKET_ALREADY_INITIALIZED, "Socket cannot be initialized again"},
    {SOCKET_RESOLVE_FAILED, "Failed to resolve host name."},
    {SOCKET_RESOLVE_BAD_FLAGS, "Invalid flags provided to resolver."},
    {SOCKET_RESOLVE_AGAIN, "Temporary failure in name resolution."},
    {SOCKET_RESOLVE_FAIL, "Non-recoverable failure in name resolution."},
    {SOCKET_RESOLVE_FAMILY, "Address family not supported for name."},
    {SOCKET_RESOLVE_SOCKTYPE, "Socket type not supported for requested address."},
    {SOCKET_RESOLVE_SERVICE, "Service not supported for socket type."},
    {SOCKET_RESOLVE_MEMORY, "Memory allocation failure during name resolution."},
    {SOCKET_RESOLVE_NO_NAME, "Name or service not known."},
    {SOCKET_RESOLVE_OVERFLOW, "Resolver result too large."},
    {UNORD_MAP_MEMORY, "Unordened map Memory allocation failed"},
    {UNORD_MAP_NOT_FOUND, "Unordened map Key not found"},
    {UNORD_MAP_UNKNOWN, "Unordened map Unknown error"},
    {DECK_EMPTY, "Deck is empty"},
    {DECK_ALLOC_FAIL, "Deck memory allocation"},
    {LOOT_TABLE_EMPTY, "Loot table is empty"},
    {SFML_WINDOW_CREATE_FAIL, "Failed to create window"},
    {CHECK_DIR_FAIL, "Check Directory: Invalid path"},
    {JSON_MALLOC_FAIL, "JSON: Malloc failure"},
    {MAP3D_ALLOC_FAIL, "Map3D allocation failure"},
    {MAP3D_OUT_OF_BOUNDS, "Map3D index out of bounds"},
    {SOCKET_JOIN_GROUP_FAILED, "Socket: Join multicast group failed"},
    {CHARACTER_INVENTORY_FULL, "Inventory full"},
    {CHARACTER_LEVEL_TABLE_INVALID, "Level table invalid"},
    {GAME_GENERAL_ERROR, "General Ingame Error"},
    {GAME_INVALID_MOVE, "Invalid Move"},
    {STACK_EMPTY, "Stack is empty"},
    {STACK_ALLOC_FAIL, "Stack memory allocation failed"},
    {QUEUE_EMPTY, "Queue is empty"},
    {QUEUE_ALLOC_FAIL, "Queue memory allocation failed"},
    {DEQUE_EMPTY, "Deque is empty"},
    {DEQUE_ALLOC_FAIL, "Deque memory allocation failed"},
    {SET_NOT_FOUND, "Set element not found"},
    {SET_ALLOC_FAIL, "Set memory allocation failed"},
    {BITSET_OUT_OF_RANGE, "Bitset index out of range"},
    {BITSET_ALLOC_FAIL, "Bitset memory allocation failed"},
    {OPTIONAL_EMPTY, "Optional has no value"},
    {OPTIONAL_ALLOC_FAIL, "Optional memory allocation failed"},
    {VARIANT_BAD_ACCESS, "Variant holds different type"},
    {VARIANT_ALLOC_FAIL, "Variant memory allocation failed"},
    {TUPLE_BAD_ACCESS, "Tuple bad access"},
    {TUPLE_ALLOC_FAIL, "Tuple memory allocation failed"},
    {PRIORITY_QUEUE_EMPTY, "Priority queue is empty"},
    {PRIORITY_QUEUE_ALLOC_FAIL, "Priority queue memory allocation failed"},
    {GRAPH_NOT_FOUND, "Graph vertex not found"},
    {GRAPH_ALLOC_FAIL, "Graph memory allocation failed"},
    {MATRIX_DIM_MISMATCH, "Matrix dimension mismatch"},
    {MATRIX_ALLOC_FAIL, "Matrix memory allocation failed"},
    {EVENT_EMITTER_NOT_FOUND, "Event emitter listener not found"},
    {EVENT_EMITTER_ALLOC_FAIL, "Event emitter memory allocation failed"},
    {CIRCULAR_BUFFER_FULL, "Circular buffer is full"},
    {CIRCULAR_BUFFER_EMPTY, "Circular buffer is empty"},
    {CIRCULAR_BUFFER_ALLOC_FAIL, "Circular buffer memory allocation failed"},
    {THREAD_POOL_FULL, "Thread pool queue is full"},
    {THREAD_POOL_ALLOC_FAIL, "Thread pool memory allocation failed"},
    {FUTURE_INVALID, "Future has no associated promise"},
    {FUTURE_ALLOC_FAIL, "Future memory allocation failed"},
    {FUTURE_BROKEN, "Associated promise was destroyed"},
    {POOL_EMPTY, "Object pool has no available entries"},
    {POOL_INVALID_OBJECT, "Object pool handle is invalid"}
};

static const char *ft_find_custom_error(int error_code)
{
    size_t error_index;
    size_t error_count;

    error_count = sizeof(g_error_strings) / sizeof(g_error_strings[0]);
    error_index = 0;
    while (error_index < error_count)
    {
        if (g_error_strings[error_index].error_code == error_code)
            return (g_error_strings[error_index].error_message);
        error_index++;
    }
    return (NULL);
}

const char* ft_strerror(int error_code)
{
    const char *custom_message;
    const char *system_message;

    custom_message = ft_find_custom_error(error_code);
    if (custom_message != NULL)
        return (custom_message);
    system_message = cmp_system_strerror(error_code);
    if (system_message != NULL)
        return (system_message);
    return ("Unrecognized error code");
}
