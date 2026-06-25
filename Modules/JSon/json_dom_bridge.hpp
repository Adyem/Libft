#ifndef JSON_DOM_BRIDGE_HPP
#define JSON_DOM_BRIDGE_HPP

#include "document.hpp"
#include "../Parser/dom.hpp"

int32_t json_document_to_dom(const json_document &document, ft_dom_document &dom) noexcept;
int32_t json_document_from_dom(const ft_dom_document &dom, json_document &document) noexcept;

#endif
