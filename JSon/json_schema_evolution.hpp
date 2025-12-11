#ifndef JSON_SCHEMA_EVOLUTION_HPP
# define JSON_SCHEMA_EVOLUTION_HPP

#include "../CPP_class/class_string.hpp"
#include "../Template/function.hpp"

class json_document;

int json_register_schema_migration(const ft_string &schema_name,
    int from_version,
    int to_version,
    const ft_function<int(json_document &)> &callback) noexcept;
int json_apply_schema_migrations(json_document &document,
    const ft_string &schema_name,
    int current_version,
    int target_version) noexcept;
int json_get_latest_schema_version(const ft_string &schema_name,
    int *out_version) noexcept;

#endif
