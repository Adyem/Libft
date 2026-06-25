# CrossProcess

The `CrossProcess` module moves descriptors and memory payload metadata between processes. It is intended for socket-based coordination where one process shares memory and another process receives enough information to read or write that memory.

## Types

- `cross_process_message` - Descriptor payload containing stack base, remote memory address/size, shared mutex and error-memory addresses, plus a fixed shared-memory name buffer.
- `cross_process_read_result` - Result of receiving shared memory. It contains an `ft_string` for the shared-memory name and an `ft_string` for the payload.

## Public API

- `cp_send_descriptor(int32_t socket_file_descriptor, const cross_process_message &message)` - Sends a descriptor message over a socket.
- `cp_receive_descriptor(int32_t socket_file_descriptor, cross_process_message &message)` - Receives a descriptor message from a socket.
- `cp_receive_memory(int32_t socket_file_descriptor, cross_process_read_result &result)` - Receives shared-memory metadata and loads the payload into `result`.
- `cp_write_memory(const cross_process_message &message, const uint8_t *payload, ft_size_t payload_length, int32_t error_code)` - Writes a payload and error code into the remote/shared memory described by a message.
