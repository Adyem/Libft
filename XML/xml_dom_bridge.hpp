#ifndef XML_DOM_BRIDGE_HPP
#define XML_DOM_BRIDGE_HPP

#include "xml.hpp"
#include "../Parser/dom.hpp"

int xml_document_to_dom(const xml_document &document, ft_dom_document &dom) noexcept;
int xml_document_from_dom(const ft_dom_document &dom, xml_document &document) noexcept;

#endif
