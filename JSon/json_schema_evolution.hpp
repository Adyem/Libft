#ifndef JSON_SCHEMA_EVOLUTION_HPP
# define JSON_SCHEMA_EVOLUTION_HPP

#include "../CPP_class/class_string.hpp"
#include "../Template/function.hpp"

class json_document;

int32_t json_register_schema_migration(const ft_string &schema_name,
    int32_t from_version,
    int32_t to_version,
    const ft_function<int32_t(json_document &)> &callback) noexcept;
int32_t json_apply_schema_migrations(json_document &document,
    const ft_string &schema_name,
    int32_t current_version,
    int32_t target_version) noexcept;
int32_t json_get_latest_schema_version(const ft_string &schema_name,
    int32_t *out_version) noexcept;

#endif
