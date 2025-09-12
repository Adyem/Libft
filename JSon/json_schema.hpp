#ifndef JSON_SCHEMA_HPP
#define JSON_SCHEMA_HPP

struct json_group;

typedef enum json_type
{
    JSON_STRING,
    JSON_NUMBER,
    JSON_BOOL
} json_type;

typedef struct json_schema_field
{
    const char *key;
    json_type   type;
    bool        required;
    struct json_schema_field *next;
} json_schema_field;

typedef struct json_schema
{
    json_schema_field *fields;
} json_schema;

bool        json_validate_schema(json_group *group, const json_schema &schema);

#endif
