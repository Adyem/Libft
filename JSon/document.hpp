#ifndef JSON_DOCUMENT_HPP
#define JSON_DOCUMENT_HPP

#include "json.hpp"

class json_document
{
    private:
        json_group *_groups;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

    public:
        json_document() noexcept;
        ~json_document() noexcept;

        json_group   *create_group(const char *name) noexcept;
        json_item    *create_item(const char *key, const char *value) noexcept;
        json_item    *create_item(const char *key, const ft_big_number &value) noexcept;
        json_item    *create_item(const char *key, const int value) noexcept;
        json_item    *create_item(const char *key, const bool value) noexcept;
        void         add_item(json_group *group, json_item *item) noexcept;
        void         append_group(json_group *group) noexcept;
        int          write_to_file(const char *file_path) const noexcept;
        int          write_to_backend(ft_document_sink &sink) const noexcept;
        char         *write_to_string() const noexcept;
        int          read_from_file(const char *file_path) noexcept;
        int          read_from_file_streaming(const char *file_path, size_t buffer_capacity) noexcept;
        int          read_from_backend(ft_document_source &source) noexcept;
        int          read_from_string(const char *content) noexcept;
        json_group   *find_group(const char *name) const noexcept;
        json_item    *find_item(json_group *group, const char *key) const noexcept;
        json_item    *find_item_by_pointer(const char *pointer) const noexcept;
        const char   *get_value_by_pointer(const char *pointer) const noexcept;
        void         remove_group(const char *name) noexcept;
        void         remove_item(json_group *group, const char *key) noexcept;
        void         update_item(json_group *group, const char *key, const char *value) noexcept;
        void         update_item(json_group *group, const char *key, const int value) noexcept;
        void         update_item(json_group *group, const char *key, const bool value) noexcept;
        void         update_item(json_group *group, const char *key, const ft_big_number &value) noexcept;
        void         clear() noexcept;
        json_group   *get_groups() const noexcept;
        void         set_manual_error(int error_code) noexcept;
        int          get_error() const noexcept;
        const char   *get_error_str() const noexcept;
};

#endif
