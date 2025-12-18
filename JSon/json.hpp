#ifndef PARSING_HPP
# define PARSING_HPP

#include "json_schema.hpp"
#include "json_stream_reader.hpp"
#include "json_stream_writer.hpp"
#include "../Parser/document_backend.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_big_number;

typedef struct json_item
{
    char *key;
    char *value;
    bool is_big_number;
    ft_big_number *big_number;
    struct json_item *next;
    mutable pt_mutex _mutex;
    mutable int _error_code;
} json_item;

typedef struct json_group
{
    char *name;
    json_item *items;
    struct json_group *next;
    mutable pt_mutex _mutex;
    mutable int _error_code;
} json_group;

class json_document;

json_item    *json_create_item(const char *key, const char *value);
json_item    *json_create_item(const char *key, const ft_big_number &value);
void         json_add_item_to_group(json_group *group, json_item *item);
json_group    *json_create_json_group(const char *name);
json_item    *json_create_item(const char *key, const int value);
json_item    *json_create_item(const char *key, const bool value);
void         json_item_refresh_numeric_state(json_item *item);
void         json_append_group(json_group **head, json_group *new_group);
int         json_write_to_file(const char *filename, json_group *groups);
int         json_write_to_backend(ft_document_sink &sink, json_group *groups);
char        *json_write_to_string(json_group *groups);
int         json_document_write_to_file(const char *file_path, const json_document &document);
int         json_document_write_to_backend(ft_document_sink &sink, const json_document &document);
char        *json_document_write_to_string(const json_document &document);
json_group  *json_read_from_file(const char *filename);
json_group  *json_read_from_backend(ft_document_source &source);
json_group  *json_read_from_string(const char *content);
json_group  *json_read_from_file_stream(FILE *file, size_t buffer_capacity);
json_group  *json_read_from_stream(json_stream_read_callback callback, void *user_data, size_t buffer_capacity);
int         json_document_read_from_backend(json_document &document, ft_document_source &source);
void         json_free_items(json_item *item);
void         json_free_groups(json_group *group);
json_group  *json_find_group(json_group *head, const char *name);
json_item   *json_find_item(json_group *group, const char *key);
void        json_remove_group(json_group **head, const char *name);
void        json_remove_item(json_group *group, const char *key);
void        json_update_item(json_group *group, const char *key, const char *value);
void        json_update_item(json_group *group, const char *key, const int value);
void        json_update_item(json_group *group, const char *key, const bool value);
void        json_update_item(json_group *group, const char *key, const ft_big_number &value);
int         json_group_enable_thread_safety(json_group *group);
int         json_group_lock(json_group *group, ft_unique_lock<pt_mutex> &guard);
void        json_group_set_error(json_group *group, int error_code);
void        json_group_set_error_unlocked(json_group *group, int error_code);
int         json_group_get_error(const json_group *group);
const char  *json_group_get_error_str(const json_group *group);
int         json_group_list_lock(ft_unique_lock<pt_mutex> &guard);
void        json_group_list_finalize_lock(ft_unique_lock<pt_mutex> &guard);
int         json_item_enable_thread_safety(json_item *item);
int         json_item_lock(json_item *item, ft_unique_lock<pt_mutex> &guard);
void        json_item_set_error(json_item *item, int error_code);
void        json_item_set_error_unlocked(json_item *item, int error_code);
int         json_item_get_error(const json_item *item);
const char  *json_item_get_error_str(const json_item *item);
int         json_schema_enable_thread_safety(json_schema *schema);
int         json_schema_lock(json_schema *schema, ft_unique_lock<pt_mutex> &guard);
void        json_schema_set_error(json_schema *schema, int error_code);
void        json_schema_set_error_unlocked(json_schema *schema, int error_code);
int         json_schema_get_error(const json_schema *schema);
const char  *json_schema_get_error_str(const json_schema *schema);
int         json_schema_field_enable_thread_safety(json_schema_field *field);
int         json_schema_field_lock(json_schema_field *field, ft_unique_lock<pt_mutex> &guard);
void        json_schema_field_set_error(json_schema_field *field, int error_code);
void        json_schema_field_set_error_unlocked(json_schema_field *field, int error_code);
int         json_schema_field_get_error(const json_schema_field *field);
const char  *json_schema_field_get_error_str(const json_schema_field *field);

#endif
