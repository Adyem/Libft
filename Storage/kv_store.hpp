#ifndef KV_STORE_HPP
#define KV_STORE_HPP

#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Errno/errno_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../JSon/json.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "../Parser/document_backend.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "../PThread/thread.hpp"
#include "../PThread/pthread.hpp"
#include "../sqlite_support.hpp"

typedef enum e_kv_store_backend_type
{
    KV_STORE_BACKEND_JSON,
    KV_STORE_BACKEND_JSON_LINES,
#if SQLITE3_AVAILABLE
    KV_STORE_BACKEND_SQLITE,
#endif
    KV_STORE_BACKEND_MEMORY_MAPPED
}   kv_store_backend_type;

extern const char *g_kv_store_ttl_prefix;

typedef struct s_kv_store_metrics
{
    long long set_operations;
    long long delete_operations;
    long long get_hits;
    long long get_misses;
    long long prune_operations;
    long long pruned_entries;
    long long total_prune_duration_ms;
    long long last_prune_duration_ms;
}   kv_store_metrics;

typedef struct s_kv_store_snapshot_entry
{
    ft_string key;
    ft_string value;
    bool has_expiration;
    long long expiration_timestamp;
}   kv_store_snapshot_entry;

class kv_store_entry
{
    private:
        ft_string _value;
        bool _has_expiration;
        long long _expiration_timestamp;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error_unlocked(int error_code) const noexcept;
        void set_error(int error_code) const noexcept;
        void reinitialize_mutex() noexcept;
        int lock_entry(ft_unique_lock<pt_mutex> &guard) const noexcept;
        static void unlock_store_guard(ft_unique_lock<pt_mutex> &guard, int error_code) noexcept;

    public:
        kv_store_entry() noexcept;
        kv_store_entry(const kv_store_entry &other) noexcept;
        kv_store_entry(kv_store_entry &&other) noexcept;
        kv_store_entry &operator=(const kv_store_entry &other) noexcept;
        kv_store_entry &operator=(kv_store_entry &&other) noexcept;
        ~kv_store_entry() noexcept;

        int set_value(const ft_string &value) noexcept;
        int set_value(const char *value_string) noexcept;
        int copy_value(ft_string &destination) const noexcept;
        int get_value_pointer(const char **value_pointer) const noexcept;

        int configure_expiration(bool has_expiration, long long expiration_timestamp) noexcept;
        int has_expiration(bool &has_expiration) const noexcept;
        int get_expiration(long long &expiration_timestamp) const noexcept;
        int is_expired(long long current_time, bool &expired) const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

typedef enum e_kv_store_operation_type
{
    KV_STORE_OPERATION_TYPE_SET,
    KV_STORE_OPERATION_TYPE_DELETE
}   kv_store_operation_type;

typedef struct s_kv_store_operation
{
    kv_store_operation_type _type;
    ft_string _key;
    ft_string _value;
    bool _has_value;
    bool _has_ttl;
    long long _ttl_seconds;
}   kv_store_operation;

typedef int (*kv_store_replication_operations_callback)(const ft_vector<kv_store_operation> &operations, void *user_data);
typedef int (*kv_store_replication_snapshot_callback)(const ft_vector<kv_store_snapshot_entry> &entries, void *user_data);

typedef struct s_kv_store_replication_sink
{
    kv_store_replication_operations_callback _operations_callback;
    kv_store_replication_snapshot_callback _snapshot_callback;
    void *_user_data;
}   kv_store_replication_sink;

class kv_store
{
    private:
        ft_map<ft_string, kv_store_entry> _data;
        ft_string _file_path;
        ft_string _encryption_key;
        bool _encryption_enabled;
        kv_store_backend_type _backend_type;
        bool _background_thread_active;
        bool _background_stop_requested;
        long long _background_interval_seconds;
        ft_thread _background_thread;
        mutable pt_mutex _background_mutex;
        mutable int _error_code;
        mutable pt_mutex _mutex;
        mutable long long _metrics_set_operations;
        mutable long long _metrics_delete_operations;
        mutable long long _metrics_get_hits;
        mutable long long _metrics_get_misses;
        mutable long long _metrics_prune_operations;
        mutable long long _metrics_pruned_entries;
        mutable long long _metrics_total_prune_duration_ms;
        mutable long long _metrics_last_prune_duration_ms;
        ft_vector<kv_store_replication_sink> _replication_sinks;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};
        mutable pt_mutex _replication_mutex;

        void set_error_unlocked(int error_code) const noexcept;
        void set_error(int error_code) const noexcept;
        int lock_store(ft_unique_lock<pt_mutex> &guard) const noexcept;
        void unlock_store_guard(ft_unique_lock<pt_mutex> &guard, int error_code) const noexcept;
        int encrypt_value(const ft_string &plain_string, ft_string &encoded_string) const;
        int decrypt_value(const ft_string &encoded_string, ft_string &plain_string) const;
        int prune_expired_locked(ft_unique_lock<pt_mutex> &guard);
        int compute_expiration(long long ttl_seconds, bool &has_expiration, long long &expiration_timestamp) const;
        long long current_time_seconds() const;
        int parse_expiration_timestamp(const char *value_string, long long &expiration_timestamp) const;
        void record_set_operation() const noexcept;
        void record_delete_operation() const noexcept;
        void record_get_hit() const noexcept;
        void record_get_miss() const noexcept;
        void record_prune_metrics(long long removed_entries, long long duration_ms) const noexcept;
        void record_operation_error(int error_code) const noexcept;
        int lock_background(ft_unique_lock<pt_mutex> &guard) const noexcept;
        int start_background_thread_locked(long long interval_seconds) noexcept;
        void stop_background_thread_locked(ft_thread &thread_holder) noexcept;
        static void background_compaction_worker(kv_store *store) noexcept;
        int apply_snapshot_locked(const ft_vector<kv_store_snapshot_entry> &entries);
        int load_json_entries(const char *location, ft_vector<kv_store_snapshot_entry> &out_entries);
        int parse_json_groups(json_group *group_head, ft_vector<kv_store_snapshot_entry> &out_entries);
        int flush_json_entries(const ft_vector<kv_store_snapshot_entry> &entries) const;
        int load_json_lines_entries(const char *location, ft_vector<kv_store_snapshot_entry> &out_entries);
        int flush_json_lines_entries(const ft_vector<kv_store_snapshot_entry> &entries) const;
#if SQLITE3_AVAILABLE
        int load_sqlite_entries(const char *location, ft_vector<kv_store_snapshot_entry> &out_entries);
        int flush_sqlite_entries(const ft_vector<kv_store_snapshot_entry> &entries) const;
#endif
        int load_memory_mapped_entries(const char *location, ft_vector<kv_store_snapshot_entry> &out_entries);
        int flush_memory_mapped_entries(const ft_vector<kv_store_snapshot_entry> &entries) const;
        int flush_backend_entries(const ft_vector<kv_store_snapshot_entry> &entries) const;
        int load_backend_entries(kv_store_backend_type backend_type, const char *location, ft_vector<kv_store_snapshot_entry> &out_entries);
        int assign_backend_location(const char *location);
        int lock_replication(ft_unique_lock<pt_mutex> &guard) const noexcept;
        int notify_replication_listeners(const ft_vector<kv_store_operation> &operations) const;
        int dispatch_snapshot_to_sink(kv_store_replication_snapshot_callback snapshot_callback, void *user_data) const;

    public:
        kv_store(const char *file_path, const char *encryption_key = ft_nullptr, bool enable_encryption = false);
        ~kv_store();

        int prune_expired();
        int kv_set(const char *key_string, const char *value_string, long long ttl_seconds = -1);
        const char *kv_get(const char *key_string) const;
        int kv_delete(const char *key_string);
        int kv_flush() const;
        int configure_encryption(const char *encryption_key, bool enable_encryption);
        int set_backend(kv_store_backend_type backend_type, const char *location);
        kv_store_backend_type get_backend() const;
        int kv_apply(const ft_vector<kv_store_operation> &operations);
        int kv_compare_and_swap(const char *key_string, const char *expected_value, const char *new_value, long long ttl_seconds = -1);
        int get_error() const;
        const char *get_error_str() const;
        pt_mutex *get_mutex_for_validation() const noexcept;
        ft_operation_error_stack *operation_error_stack_handle() const noexcept;
        int get_metrics(kv_store_metrics &out_metrics) const;
        int export_snapshot(ft_vector<kv_store_snapshot_entry> &out_entries) const;
        int export_snapshot_to_file(const char *file_path) const;
        int import_snapshot(const ft_vector<kv_store_snapshot_entry> &entries);
        int write_snapshot(ft_document_sink &sink) const;
        int read_snapshot(ft_document_source &source);
        int start_background_compaction(long long interval_seconds);
        int stop_background_compaction();
        int register_replication_sink(kv_store_replication_operations_callback operations_callback,
            kv_store_replication_snapshot_callback snapshot_callback, void *user_data,
            bool ship_initial_snapshot);
        int unregister_replication_sink(kv_store_replication_operations_callback operations_callback,
            kv_store_replication_snapshot_callback snapshot_callback, void *user_data);
        int ship_replication_snapshot(kv_store_replication_snapshot_callback snapshot_callback, void *user_data) const;
};

int kv_store_init_set_operation(kv_store_operation &operation, const char *key_string, const char *value_string, long long ttl_seconds = -1);
int kv_store_init_delete_operation(kv_store_operation &operation, const char *key_string);


#endif
