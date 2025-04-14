#ifndef PARSING_HPP
# define PARSING_HPP

typedef struct json_item
{
    char *key;
    char *value;
    struct json_item *next;
} json_item;

typedef struct json_group
{
    char *name;
    json_item *items;
    struct json_group *next;
} json_group;

json_item	*json_create_item(const char *key, const char *value);
void 		json_add_item_to_group(json_group *group, json_item *item);
json_group	*json_create_json_group(const char *name);
void 		json_append_group(json_group **head, json_group *new_group);
int 		json_write_to_file(const char *filename, json_group *groups);
void 		json_free_items(json_item *item);
void 		json_free_groups(json_group *group);

#endif
