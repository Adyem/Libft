# Sink

The `Sink` module stores formatted log messages in a simple in-memory record list. It is intentionally small and dependency-light so higher-level logging code can depend on it without pulling in the rest of the logger stack.

## Public API
- `sink_record_message(int32_t level, const char *message)` - Stores one formatted log message and its numeric level.
- `sink_get_record_count(ft_size_t *count_out)` - Returns the number of stored messages.
- `sink_get_record(ft_size_t index, sink_record *record_out)` - Retrieves one stored message by index.
- `sink_clear()` - Frees all stored messages.

## Types
- `sink_record` - Stored log message entry with numeric level and message text.
