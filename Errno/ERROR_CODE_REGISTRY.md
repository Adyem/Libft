# Libft Error Code Registry

This registry enumerates every error code defined in [`Errno/errno.hpp`](errno.hpp) and summarizes the modules that are expected to set each value. All codes are relative to `ERRNO_OFFSET` (2000) when mirroring `errno` symbols. Helpers must restore `FT_ERR_SUCCESSS` after success paths to avoid leaking stale failures.

| Code | Value | Summary | Primary modules |
| --- | --- | --- | --- |
| `FT_ERR_SUCCESSS` | 0 | Operation completed successfully. | All modules |
| `FT_ERR_NO_MEMORY` | 1 | Memory allocation failed or resources were exhausted. | Libft allocation helpers, CMA allocators, containers |
| `FT_ERR_INVALID_ARGUMENT` | 2 | Arguments failed validation (null pointers, invalid flags, etc.). | Libft string/memory helpers, System_utils, Networking |
| `FT_ERR_INVALID_POINTER` | 3 | Pointer values were malformed or not owned by the caller. | Libft memory helpers, CMA guard checks |
| `FT_ERR_INVALID_HANDLE` | 4 | Handle-based API received an invalid descriptor/object. | File module, Networking sockets, System_utils handles |
| `FT_ERR_INVALID_OPERATION` | 5 | Requested action violates API preconditions or invariants. | Libft algorithms, CMA stateful helpers |
| `FT_ERR_INVALID_STATE` | 6 | Component was not initialized or is already shut down. | CMA allocator toggles, Logger, Template task utilities |
| `FT_ERR_NOT_FOUND` | 7 | Item lookup failed. | Storage KV store, Template containers, JSON/YAML lookups |
| `FT_ERR_ALREADY_EXISTS` | 8 | Resource already present, preventing creation. | Filesystem helpers, Storage, Networking |
| `FT_ERR_OUT_OF_RANGE` | 9 | Index or numeric value outside accepted range. | Math utilities, Template containers, Libft parsing |
| `FT_ERR_EMPTY` | 10 | Operation requires data but the container/resource is empty. | Template containers, Queue helpers |
| `FT_ERR_FULL` | 11 | Container cannot accept more data. | Template circular buffer, CMA bounded allocators |
| `FT_ERR_IO` | 12 | General I/O failure. | File module, System_utils file helpers, Networking streams |
| `FT_ERR_SYSTEM` | 13 | System-level error propagated from platform `errno`. | System_utils, Compatebility wrappers |
| `FT_ERR_OVERLAP` | 14 | Source and destination buffers overlap when not allowed. | Libft memcpy/memmove safe wrappers |
| `FT_ERR_TERMINATED` | 15 | Operation aborted due to cancellation or shutdown. | Thread pools, Task schedulers, Networking shutdown |
| `FT_ERR_INTERNAL` | 16 | Internal invariant violated (should not happen). | All modules (guarded assertions) |
| `FT_ERR_CONFIGURATION` | 17 | Configuration invalid or missing required fields. | Config module, Logger, Networking setup |
| `FT_ERR_UNSUPPORTED_TYPE` | 18 | Requested type or algorithm not supported. | Serialization helpers, Compression |
| `FT_ERR_ALREADY_INITIALIZED` | 19 | Component already initialized. | CMA allocator toggles, Logger, Networking sockets |
| `FT_ERR_INITIALIZATION_FAILED` | 20 | Initialization routine failed. | Logger, Networking, RNG |
| `FT_ERR_END_OF_FILE` | 21 | Reached end-of-file or stream. | File and ReadLine modules |
| `FT_ERR_DIVIDE_BY_ZERO` | 22 | Division by zero attempted. | Math helpers |
| `FT_ERR_BROKEN_PROMISE` | 23 | Promise or future was abandoned. | Template promise/future utilities |
| `FT_ERR_SOCKET_CREATION_FAILED` | 24 | `socket()`/platform equivalent failed. | Networking |
| `FT_ERR_SOCKET_BIND_FAILED` | 25 | Socket bind operation failed. | Networking |
| `FT_ERR_SOCKET_LISTEN_FAILED` | 26 | Socket listen setup failed. | Networking |
| `FT_ERR_SOCKET_CONNECT_FAILED` | 27 | Connect attempt failed. | Networking |
| `FT_ERR_NETWORK_CONNECT_FAILED` | 28 | Network connection failed after transport setup. | Networking higher-level connectors |
| `FT_ERR_INVALID_IP_FORMAT` | 29 | IP address parsing failed. | Networking |
| `FT_ERR_SOCKET_ACCEPT_FAILED` | 30 | `accept()` failed. | Networking |
| `FT_ERR_SOCKET_SEND_FAILED` | 31 | Sending data failed. | Networking |
| `FT_ERR_SOCKET_RECEIVE_FAILED` | 32 | Receiving data failed. | Networking |
| `FT_ERR_SOCKET_CLOSE_FAILED` | 33 | Socket close failed. | Networking |
| `FT_ERR_SOCKET_JOIN_GROUP_FAILED` | 34 | Multicast group join failed. | Networking |
| `FT_ERR_SOCKET_RESOLVE_FAILED` | 35 | Hostname resolution failed (generic). | Networking DNS helpers |
| `FT_ERR_SOCKET_RESOLVE_BAD_FLAGS` | 36 | Invalid flags passed to resolver. | Networking DNS helpers |
| `FT_ERR_SOCKET_RESOLVE_AGAIN` | 37 | Resolver requested retry (`EAI_AGAIN`). | Networking DNS helpers |
| `FT_ERR_SOCKET_RESOLVE_FAIL` | 38 | Non-retriable resolve failure (`EAI_FAIL`). | Networking DNS helpers |
| `FT_ERR_SOCKET_RESOLVE_FAMILY` | 39 | Unsupported address family requested. | Networking DNS helpers |
| `FT_ERR_SOCKET_RESOLVE_SOCKTYPE` | 40 | Unsupported socket type requested. | Networking DNS helpers |
| `FT_ERR_SOCKET_RESOLVE_SERVICE` | 41 | Service/port lookup failed. | Networking DNS helpers |
| `FT_ERR_SOCKET_RESOLVE_MEMORY` | 42 | Resolver out of memory. | Networking DNS helpers |
| `FT_ERR_SOCKET_RESOLVE_NO_NAME` | 43 | Host/service not found (`EAI_NONAME`). | Networking DNS helpers |
| `FT_ERR_SOCKET_RESOLVE_OVERFLOW` | 44 | Resolver buffer overflow (`EAI_OVERFLOW`). | Networking DNS helpers |
| `FT_ERR_GAME_GENERAL_ERROR` | 45 | Generic game engine failure. | Game module |
| `FT_ERR_GAME_INVALID_MOVE` | 46 | Invalid game move or rule violation. | Game module |
| `FT_ERR_THREAD_BUSY` | 47 | Thread cannot accept additional work. | PThread and Template threading helpers |
| `FT_ERR_MUTEX_NOT_OWNER` | 48 | Mutex unlocked by non-owner. | PThread mutex helpers, Template synchronization |
| `FT_ERR_MUTEX_ALREADY_LOCKED` | 49 | Mutex already locked or recursive violation. | PThread mutex helpers, Template synchronization |
| `FT_ERR_SSL_WANT_READ` | 50 | SSL layer needs more data to read. | Networking TLS wrappers |
| `FT_ERR_SSL_WANT_WRITE` | 51 | SSL layer needs to flush writes. | Networking TLS wrappers |
| `FT_ERR_SSL_ZERO_RETURN` | 52 | SSL connection closed cleanly. | Networking TLS wrappers |
| `FT_ERR_BITSET_NO_MEMORY` | 53 | Bitset allocation failed. | Template bitset |
| `FT_ERR_PRIORITY_QUEUE_EMPTY` | 54 | Priority queue pop on empty container. | Template priority queue |
| `FT_ERR_PRIORITY_QUEUE_NO_MEMORY` | 55 | Priority queue allocation failed. | Template priority queue |
| `FT_ERR_CRYPTO_INVALID_PADDING` | 56 | Cryptographic operation encountered invalid padding. | Encryption module |
| `FT_ERR_SSL_SYSCALL_ERROR` | 2005 | SSL syscall failure propagated from platform `errno`. | Networking TLS wrappers |
| `FT_ERR_HTTP_PROTOCOL_MISMATCH` | 2006 | Server replied with a different HTTP version than requested. | API HTTP clients |
| `FT_ERR_API_CIRCUIT_OPEN` | 2007 | API circuit breaker prevented request execution. | API HTTP clients |

## Usage guidance

* When adding new error codes, update this registry and provide guidance on the modules that surface the value.
* Modules should document code-specific remediation steps in their README entries and keep error propagation consistent with this table.
* Tests should exercise both success and failure paths to ensure `ft_errno` is set and cleared as described here.
