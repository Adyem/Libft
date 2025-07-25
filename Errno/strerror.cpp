#include "errno.hpp"
#include <cerrno>
#include <cstring>

const char* ft_strerror(int error_code)
{
	if (error_code == ER_SUCCESS)
        return ("Operation successful");
	else if (error_code == CMA_BAD_ALLOC)
		return ("Bad allocation");
	else if (error_code == CMA_INVALID_PTR)
		return ("Invalid CMA pointer");
	else if (error_code == PT_ERR_QUEUE_FULL)
        return ("Wait queue is full");
	else if (error_code == PT_ERR_MUTEX_NULLPTR)
        return ("Mutex pointer is null");
	else if (error_code == PT_ERR_MUTEX_OWNER)
        return ("Thread is not the owner of the mutex");
	else if (error_code == PT_ERR_ALRDY_LOCKED)
        return ("Thread already locked the mutex");
	else if (error_code == PT_ALREADDY_LOCKED)
		return ("Another thread alreaddy locked the mutex");
	else if (error_code == SHARED_PTR_NULL_PTR)
        return ("Null pointer dereference");
	else if (error_code == SHARED_PTR_OUT_OF_BOUNDS)
        return ("Array index out of bounds");
	else if (error_code == SHARED_PTR_ALLOCATION_FAILED)
        return ("Shared pointer allocation failed");
	else if (error_code == SHARED_PTR_INVALID_OPERATION)
        return ("Invalid operation on shared pointer");
	else if (error_code == SHARED_PTR_ELEMENT_ALREADDY_ADDED)
		return ("Element alreaddy on the array");
	else if (error_code == MAP_ALLOCATION_FAILED)
        return ("Map memory allocation failed");
	else if (error_code == MAP_KEY_NOT_FOUND)
        return ("Map key not found");
	else if (error_code == FILE_INVALID_FD)
        return ("Bad file descriptor");
	else if (error_code == FT_EINVAL)
        return ("Invalid argument");
	else if (error_code == STRING_MEM_ALLOC_FAIL)
        return ("String memory allocation failed");
	else if (error_code == STRING_ERASE_OUT_OF_BOUNDS)
		return ("String acces out of bounds");
	else if (error_code == VECTOR_ALLOC_FAIL)
        return ("Vector memory allocation failed");
	else if (error_code == VECTOR_OUT_OF_BOUNDS)
        return ("Vector index out of bounds");
	else if (error_code == VECTOR_INVALID_PTR)
        return ("Vector invalid pointer");
	else if (error_code == VECTOR_CRITICAL_ERROR)
        return ("Vector critical error");
	else if (error_code == VECTOR_INVALID_OPERATION)
        return ("Vector invalid operation");
	else if (error_code == FT_EALLOC)
		return ("Allocation error");
	else if (error_code == FT_ETERM)
		return ("Terminal error");
	else if (error_code == SOCKET_CREATION_FAILED)
        return ("Failed to create socket.");
	else if (error_code == SOCKET_BIND_FAILED)
        return ("Failed to bind socket.");
	else if (error_code == SOCKET_LISTEN_FAILED)
        return ("Failed to listen on socket.");
	else if (error_code == SOCKET_CONNECT_FAILED)
        return ("Failed to connect to server.");
	else if (error_code == INVALID_IP_FORMAT)
        return ("Invalid IP address format.");
	else if (error_code == UNSUPPORTED_SOCKET_TYPE)
        return ("Unsupported socket type.");
	else if (error_code == SOCKET_ACCEPT_FAILED)
        return ("Failed to accept connection.");
	else if (error_code == SOCKET_SEND_FAILED)
        return ("Failed to send data through socket.");
	else if (error_code == SOCKET_RECEIVE_FAILED)
		return ("Failed to receive data from socket.");
	else if (error_code == SOCKET_CLOSE_FAILED)
        return ("Failed to close socket.");
	else if (error_code == SOCKET_INVALID_CONFIGURATION)
		return ("Socket invalid configuration");
	else if (error_code == SOCKET_UNSUPPORTED_TYPE)
		return ("Socket unsupported type");
	else if (error_code == SOCKET_ALRDY_INITIALIZED)
		return ("Socket cannot be initialized again");
	else if (error_code == UNORD_MAP_MEMORY)
		return ("Unordened map Memory allocation failed");
	else if (error_code == UNORD_MAP_NOT_FOUND)
		return ("Unordened map Key not found");
	else if (error_code == UNORD_MAP_UNKNOWN)
		return ("Unordened map Unknown error");
	else if (error_code == DECK_EMPTY)
		return ("Deck is empty");
	else if (error_code == DECK_ALLOC_FAIL)
		return ("Deck memory allocation");
	else if (error_code == SFML_WINDOW_CREATE_FAIL)
		return ("Failed to create window");
	else if (error_code == CHECK_DIR_FAIL)
		return ("Check Directory: Invalid path");
	else if (error_code == JSON_MALLOC_FAIL)
		return ("JSON: Malloc failure");
	else if (error_code == MAP3D_ALLOC_FAIL)
		return ("Map3D allocation failure");
	else if (error_code == MAP3D_OUT_OF_BOUNDS)
		return ("Map3D index out of bounds");
	else if (error_code == SOCKET_JOIN_GROUP_FAILED)
		return ("Socket: Join multicast group failed");
	else if (error_code == CHARACTER_INVENTORY_FULL)
		return ("Inventory full");
	else if (error_code == CHARACTER_LEVEL_TABLE_INVALID)
		return ("Level table invalid");
	else if (error_code == GAME_GENERAL_ERROR)
		return ("General Ingame Error");
	else if (error_code == GAME_INVALID_MOVE)
		return ("Invalid Move");
	else if (error_code > ERRNO_OFFSET)
        {
        int standard_errno = error_code - ERRNO_OFFSET;
        const char *message = strerror(standard_errno);
        if (message)
            return (message);
        return ("Unrecognized error code");
	}
	else
        return ("Unrecognized error code");
}
