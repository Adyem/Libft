#ifndef YAML_DOM_BRIDGE_HPP
#define YAML_DOM_BRIDGE_HPP

#include "yaml.hpp"
#include "../Parser/dom.hpp"

yaml_value *yaml_value_from_dom(const ft_dom_document &dom) noexcept;
int yaml_value_to_dom(const yaml_value *value, ft_dom_document &dom) noexcept;

#endif
