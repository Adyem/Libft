#ifndef XML_HPP
#define XML_HPP

#include "xml_document.hpp"

typedef int32_t (*xml_serialize_callback)(
    xml_document &document, void *user_data) noexcept;
typedef int32_t (*xml_deserialize_callback)(
    const xml_document &document, void *user_data) noexcept;

int32_t xml_serialize_to_string(xml_serialize_callback serialize_callback,
    void *user_data, ft_string &output) noexcept;
int32_t xml_serialize_to_backend(xml_serialize_callback serialize_callback,
    void *user_data, ft_document_sink &sink) noexcept;
int32_t xml_deserialize_from_string(const char *content,
    xml_deserialize_callback deserialize_callback, void *user_data) noexcept;
int32_t xml_deserialize_from_backend(ft_document_source &source,
    xml_deserialize_callback deserialize_callback, void *user_data) noexcept;

#endif
