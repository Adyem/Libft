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

#endif
