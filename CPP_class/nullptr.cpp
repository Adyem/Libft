#include "nullptr.hpp"

namespace ft
{
	const nullptr_t ft_nullptr_instance [[maybe_unused]] = nullptr_t{};

	[[maybe_unused]] void nullptr_t::operator&() const {}

}
