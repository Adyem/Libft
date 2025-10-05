#ifndef KV_STORE_HPP
#define KV_STORE_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../JSon/json.hpp"
#include "../Template/map.hpp"
#include "../CPP_class/class_string_class.hpp"

class kv_store
{
    private:
        ft_map<ft_string, ft_string> _data;
        ft_string _file_path;
        ft_string _encryption_key;
        bool _encryption_enabled;
        mutable int _error_code;

        void set_error(int error_code) const;
        int encrypt_value(const ft_string &plain_string, ft_string &encoded_string) const;
        int decrypt_value(const ft_string &encoded_string, ft_string &plain_string) const;

    public:
        kv_store(const char *file_path, const char *encryption_key = ft_nullptr, bool enable_encryption = false);
        ~kv_store();

        int kv_set(const char *key_string, const char *value_string);
        const char *kv_get(const char *key_string) const;
        int kv_delete(const char *key_string);
        int kv_flush() const;
        int configure_encryption(const char *encryption_key, bool enable_encryption);
        int get_error() const;
        const char *get_error_str() const;
};

#endif
