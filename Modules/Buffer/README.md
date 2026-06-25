# Buffer

The `Buffer` module provides `ft_byte_buffer`, a lifecycle-managed byte buffer with append/read cursors, endian-aware integer helpers, variable-length integer helpers, search/slice helpers, fixed-capacity mode, optional thread safety, and object-local error reporting.

## `ft_byte_buffer`

### Lifecycle

- `ft_byte_buffer()` - Creates an uninitialized buffer object.
- `~ft_byte_buffer()` - Releases owned storage during destruction.
- `initialize()` - Initializes an empty growable buffer.
- `initialize(ft_size_t capacity, ft_bool fixed_capacity)` - Initializes storage with a requested capacity and optionally prevents growth.
- `initialize(const ft_byte_buffer &other)` - Copies another initialized buffer into this object.
- `initialize(ft_byte_buffer &&other)` - Move-initializes this object from another buffer.
- `destroy()` - Releases storage and resets the lifecycle state.
- `move(ft_byte_buffer &other)` - Explicitly transfers state from another initialized buffer.

### Buffer Operations

- `reserve(ft_size_t required_capacity)` - Ensures at least the requested capacity is available.
- `clear()` - Removes buffered data and resets read state.
- `reset_read_position()` - Moves the read cursor back to the start.
- `set_read_position(ft_size_t position)` - Sets the read cursor to an absolute offset.
- `consume(ft_size_t length)` - Drops bytes from the front of the buffer.
- `append(const void *data, ft_size_t length)` - Appends raw bytes.
- `append_buffer(const ft_byte_buffer &other)` - Appends the stored bytes from another buffer.
- `prepend_buffer(const ft_byte_buffer &other)` - Prepends the stored bytes from another buffer and keeps the read cursor aligned with the original unread data.
- `read(void *data, ft_size_t length)` - Copies bytes from the current read cursor and advances it.
- `view(ft_size_t offset, ft_size_t length, const uint8_t **view_out)` - Returns a read-only pointer into a valid buffer range.
- `shrink_to_fit()` - Releases unused capacity so the allocation matches the stored size.

### Endian-Aware Integer Helpers

- `append_u8(uint8_t value)` - Appends one byte.
- `append_u16_be(uint16_t value)` / `append_u16_le(uint16_t value)` - Appends a 16-bit integer in big- or little-endian order.
- `append_u32_be(uint32_t value)` / `append_u32_le(uint32_t value)` - Appends a 32-bit integer in big- or little-endian order.
- `append_u64_be(uint64_t value)` / `append_u64_le(uint64_t value)` - Appends a 64-bit integer in big- or little-endian order.
- `append_varuint64(uint64_t value)` - Appends an unsigned LEB128-encoded integer.
- `append_varint64(int64_t value)` - Appends a signed LEB128-encoded integer.
- `read_u8(uint8_t *value_out)` - Reads one byte.
- `read_u16_be(uint16_t *value_out)` / `read_u16_le(uint16_t *value_out)` - Reads a 16-bit integer in big- or little-endian order.
- `read_u32_be(uint32_t *value_out)` / `read_u32_le(uint32_t *value_out)` - Reads a 32-bit integer in big- or little-endian order.
- `read_u64_be(uint64_t *value_out)` / `read_u64_le(uint64_t *value_out)` - Reads a 64-bit integer in big- or little-endian order.
- `read_varuint64(uint64_t *value_out)` - Reads an unsigned LEB128-encoded integer.
- `read_varint64(int64_t *value_out)` - Reads a signed LEB128-encoded integer.

### Search and Slice Helpers

- `find(const void *data, ft_size_t length)` - Finds a byte sequence from the current read cursor and returns the absolute offset.
- `peek(ft_size_t offset, void *data, ft_size_t length)` - Copies bytes from an absolute offset without advancing the read cursor.
- `skip(ft_size_t length)` - Advances the read cursor without copying data.
- `slice(ft_size_t offset, ft_size_t length, ft_byte_buffer &output)` - Copies a byte range into another buffer.

### State and Thread Safety

- `data()` - Returns the beginning of the stored byte range.
- `size()` - Returns the number of bytes currently stored.
- `capacity()` - Returns the allocated capacity.
- `read_position()` - Returns the current read cursor offset.
- `remaining()` - Returns bytes available from the current read cursor.
- `is_fixed_capacity()` - Reports whether the buffer is allowed to grow.
- `is_initialised()` - Reports whether the object is initialized.
- `enable_thread_safety()` - Allocates and enables the optional recursive mutex.
- `disable_thread_safety()` - Releases the optional mutex.
- `is_thread_safe()` - Reports whether the optional mutex is enabled.
- `get_error()` - Returns the buffer's last error code.
- `get_error_str()` - Returns readable text for the buffer's last error.
