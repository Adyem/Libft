#ifndef KV_STORE_HPP
#define KV_STORE_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../JSon/json.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../Template/map.hpp"
#include "../Template/vector.hpp"

extern const char *g_kv_store_ttl_prefix;

typedef struct s_kv_store_entry
{
    ft_string _value;
    bool _has_expiration;
    long long _expiration_timestamp;
}   kv_store_entry;

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

class kv_store
{
    private:
        ft_map<ft_string, kv_store_entry> _data;
        ft_string _file_path;
        ft_string _encryption_key;
        bool _encryption_enabled;
        mutable int _error_code;

        void set_error(int error_code) const;
        int encrypt_value(const ft_string &plain_string, ft_string &encoded_string) const;
        int decrypt_value(const ft_string &encoded_string, ft_string &plain_string) const;
        int prune_expired();
        int compute_expiration(long long ttl_seconds, bool &has_expiration, long long &expiration_timestamp) const;
        long long current_time_seconds() const;
        int parse_expiration_timestamp(const char *value_string, long long &expiration_timestamp) const;

    public:
        kv_store(const char *file_path, const char *encryption_key = ft_nullptr, bool enable_encryption = false);
        ~kv_store();

        int kv_set(const char *key_string, const char *value_string, long long ttl_seconds = -1);
        const char *kv_get(const char *key_string) const;
        int kv_delete(const char *key_string);
        int kv_flush() const;
        int configure_encryption(const char *encryption_key, bool enable_encryption);
        int kv_apply(const ft_vector<kv_store_operation> &operations);
        int kv_compare_and_swap(const char *key_string, const char *expected_value, const char *new_value, long long ttl_seconds = -1);
        int get_error() const;
        const char *get_error_str() const;
};

int kv_store_init_set_operation(kv_store_operation &operation, const char *key_string, const char *value_string, long long ttl_seconds = -1);
int kv_store_init_delete_operation(kv_store_operation &operation, const char *key_string);


#endif
