#ifndef KV_STORE_HPP
#define KV_STORE_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../JSon/json.hpp"
#include <map>
#include <string>

class kv_store
{
    private:
        std::map<std::string, std::string> _data;
        std::string _file_path;
        mutable int _error_code;

        void set_error(int error_code) const;

    public:
        kv_store(const char *file_path);
        ~kv_store();

        int kv_set(const char *key_string, const char *value_string);
        const char *kv_get(const char *key_string) const;
        int kv_delete(const char *key_string);
        int kv_flush() const;
        int get_error() const;
        const char *get_error_str() const;
};

#endif
