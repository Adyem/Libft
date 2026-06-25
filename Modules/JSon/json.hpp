#ifndef PARSING_HPP
# define PARSING_HPP

#include "json_schema.hpp"
#include "json_stream_reader.hpp"
#include "json_stream_writer.hpp"
#include "../Parser/document_backend.hpp"
#include "../Advanced/advanced.hpp"
#include "../PThread/recursive_mutex.hpp"

class ft_big_number;

typedef struct json_item
{
    char *key;
    char *value;
    ft_bool is_big_number;
    ft_big_number *big_number;
    struct json_item *next;
    mutable pt_recursive_mutex *_mutex;
    mutable int32_t _error_code;
} json_item;

typedef struct json_group
{
    char *name;
    json_item *items;
    struct json_group *next;
    mutable pt_recursive_mutex *_mutex;
    mutable int32_t _error_code;
} json_group;

class json_document;

json_item    *json_create_item(const char *key, const char *value);
json_item    *json_create_item(const char *key, const ft_big_number &value);
void         json_add_item_to_group(json_group *group, json_item *item);
json_group    *json_create_json_group(const char *name);
json_item    *json_create_item(const char *key, const int32_t value);
json_item    *json_create_item(const char *key, const ft_bool value);
void         json_item_refresh_numeric_state(json_item *item);
void         json_append_group(json_group **head, json_group *new_group);
int32_t         json_write_to_file(const char *filename, json_group *groups);
int32_t         json_write_to_backend(ft_document_sink &sink, json_group *groups);
char        *json_write_to_string(json_group *groups);
int32_t         json_document_write_to_file(const char *file_path, const json_document &document);
int32_t         json_document_write_to_backend(ft_document_sink &sink, const json_document &document);
char        *json_document_write_to_string(const json_document &document);
json_group  *json_read_from_file(const char *filename);
json_group  *json_read_from_backend(ft_document_source &source);
json_group  *json_read_from_string(const char *content);
json_group  *json_read_from_file_stream(FILE *file, ft_size_t buffer_capacity);
json_group  *json_read_from_stream(json_stream_read_callback callback, void *user_data, ft_size_t buffer_capacity);
int32_t         json_document_read_from_backend(json_document &document, ft_document_source &source);
void         json_free_items(json_item *item);
void         json_free_groups(json_group *group);
json_group  *json_find_group(json_group *head, const char *name);
json_item   *json_find_item(json_group *group, const char *key);
void        json_remove_group(json_group **head, const char *name);
void        json_remove_item(json_group *group, const char *key);
void        json_update_item(json_group *group, const char *key, const char *value);
void        json_update_item(json_group *group, const char *key, const int32_t value);
void        json_update_item(json_group *group, const char *key, const ft_bool value);
void        json_update_item(json_group *group, const char *key, const ft_big_number &value);
int32_t         json_group_enable_thread_safety(json_group *group);
int32_t         json_group_disable_thread_safety(json_group *group);
ft_bool        json_group_is_thread_safe(const json_group *group);
int32_t         json_group_lock(json_group *group);
void        json_group_set_error(json_group *group, int32_t error_code);
void        json_group_set_error_unlocked(json_group *group, int32_t error_code);
int32_t         json_group_get_error(const json_group *group);
const char  *json_group_get_error_str(const json_group *group);
int32_t         json_group_list_lock_manual();
int32_t         json_group_list_unlock_manual();
int32_t         json_group_list_lock();
void        json_group_list_finalize_lock();
int32_t         json_group_list_enable_thread_safety();
int32_t         json_group_list_disable_thread_safety();
int32_t         json_item_enable_thread_safety(json_item *item);
int32_t         json_item_disable_thread_safety(json_item *item);
ft_bool        json_item_is_thread_safe(const json_item *item);
int32_t         json_item_lock(json_item *item);
void        json_item_set_error(json_item *item, int32_t error_code);
void        json_item_set_error_unlocked(json_item *item, int32_t error_code);
int32_t         json_item_get_error(const json_item *item);
const char  *json_item_get_error_str(const json_item *item);
int32_t         json_schema_enable_thread_safety(json_schema *schema);
int32_t         json_schema_disable_thread_safety(json_schema *schema);
ft_bool        json_schema_is_thread_safe(const json_schema *schema);
int32_t         json_schema_lock(json_schema *schema);
void        json_schema_set_error(json_schema *schema, int32_t error_code);
void        json_schema_set_error_unlocked(json_schema *schema, int32_t error_code);
int32_t         json_schema_get_error(const json_schema *schema);
const char  *json_schema_get_error_str(const json_schema *schema);
int32_t         json_schema_field_enable_thread_safety(json_schema_field *field);
int32_t         json_schema_field_disable_thread_safety(json_schema_field *field);
ft_bool        json_schema_field_is_thread_safe(const json_schema_field *field);
int32_t         json_schema_field_lock(json_schema_field *field);
void        json_schema_field_set_error(json_schema_field *field, int32_t error_code);
void        json_schema_field_set_error_unlocked(json_schema_field *field, int32_t error_code);
int32_t         json_schema_field_get_error(const json_schema_field *field);
const char  *json_schema_field_get_error_str(const json_schema_field *field);

int32_t         json_stream_reader_disable_thread_safety(json_stream_reader *reader);
ft_bool        json_stream_reader_is_thread_safe(const json_stream_reader *reader);

typedef int32_t (*json_serialize_callback)(
    json_document &document, void *user_data) noexcept;
typedef int32_t (*json_deserialize_callback)(
    const json_document &document, void *user_data) noexcept;

int32_t json_serialize_to_string(json_serialize_callback serialize_callback,
    void *user_data, ft_string &output) noexcept;
int32_t json_serialize_to_backend(json_serialize_callback serialize_callback,
    void *user_data, ft_document_sink &sink) noexcept;
int32_t json_deserialize_from_string(const char *content,
    json_deserialize_callback deserialize_callback, void *user_data) noexcept;
int32_t json_deserialize_from_backend(ft_document_source &source,
    json_deserialize_callback deserialize_callback, void *user_data) noexcept;

#endif
