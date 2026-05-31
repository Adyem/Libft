# Storage

The `Storage` module provides `kv_store`, a lifecycle key/value database with JSON, JSON-lines, SQLite when available, and memory-mapped backends. It supports TTL expiration, optional encryption, snapshots, background compaction, metrics, and replication callbacks. Its background workers use the separate `Threading` module.

## Types

- `kv_store_backend_type` - Backend enum for JSON, JSON lines, SQLite when compiled in, and memory-mapped storage.
- `g_kv_store_ttl_prefix` - Prefix used internally to encode TTL metadata.
- `kv_store_metrics` - Counters for set/delete/get/prune operations and prune duration.
- `kv_store_snapshot_entry` - Snapshot record containing key, value, expiration flag, and expiration timestamp. It exposes `initialize`, move initialization, and `destroy`.
- `kv_store_entry` - Lifecycle value entry containing stored value plus optional expiration.
- `kv_store_operation_type` - Replication/apply operation kind: set or delete.
- `kv_store_operation` - Batched operation with type, key, value, value flag, TTL flag, and TTL seconds. It exposes `initialize`, move initialization, and `destroy`.
- `kv_store_replication_sink` - Operations callback, snapshot callback, and user data.

## `kv_store_entry`

- Lifecycle and thread-safety methods - `initialize`, copy/move initialization, `destroy`, `move`, `enable_thread_safety`, `disable_thread_safety`, and `is_thread_safe`.
- `set_value(const ft_string &value)` / `set_value(const char *value_string)` - Store a new value.
- `copy_value(ft_string &destination)` - Copies the stored value into a destination string.
- `get_value_pointer(const char **value_pointer)` - Returns a pointer to the stored C string.
- `configure_expiration(ft_bool has_expiration, int64_t expiration_timestamp)` - Sets expiration metadata.
- `has_expiration(...)` / `get_expiration(...)` - Read expiration metadata.
- `is_expired(int64_t current_time, ft_bool &expired)` - Reports whether the entry is expired at a timestamp.

## `kv_store`

- Lifecycle and thread-safety methods - constructor, destructor, `initialize`, copy/move initialization, `destroy`, `move`, `enable_thread_safety`, `disable_thread_safety`, and `is_thread_safe`.
- `initialize(const char *file_path, const char *encryption_key, ft_bool enable_encryption)` - Opens or creates the store at a location.
- `kv_set(...)` - Stores a key/value pair with optional TTL seconds.
- `kv_get(...)` - Returns a stored value pointer or null on miss.
- `kv_delete(...)` - Deletes a key.
- `kv_flush()` - Writes current data to the configured backend.
- `prune_expired()` - Removes expired entries.
- `configure_encryption(...)` - Updates encryption key and enable flag.
- `set_backend(...)` / `get_backend()` - Configure or inspect the active backend.
- `kv_apply(const ft_vector<kv_store_operation> &operations)` - Applies a batch of set/delete operations.
- `kv_compare_and_swap(...)` - Replaces a value only when the current value matches an expected value.
- `get_metrics(kv_store_metrics &out_metrics)` - Writes operation counters.
- `export_snapshot(...)` / `import_snapshot(...)` - Move store contents to or from snapshot entries.
- `export_snapshot_to_file(...)` - Writes a snapshot to a file.
- `write_snapshot(ft_document_sink &sink)` / `read_snapshot(ft_document_source &source)` - Use abstract document IO for snapshots.
- `start_background_compaction(...)` / `stop_background_compaction()` - Manage periodic prune/flush work.
- `register_replication_sink(...)` / `unregister_replication_sink(...)` - Add or remove replication listeners.
- `ship_replication_snapshot(...)` - Sends the current snapshot to a callback.

## Operation Helpers

- `kv_store_init_set_operation(...)` - Initializes a set operation.
- `kv_store_init_delete_operation(...)` - Initializes a delete operation.
