#include "../../Template/shared_ptr.hpp"
#include "../../Template/unique_ptr.hpp"
#include "../../Template/template_concepts.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"

class variadic_constructible
{
    private:
        int _first;
        double _second;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        variadic_constructible();
        variadic_constructible(int first, double second);
        variadic_constructible(const variadic_constructible &other);
        variadic_constructible &operator=(const variadic_constructible &other);
        ~variadic_constructible();

        int first() const;
        double second() const;
        int get_error() const;
        const char *get_error_str() const;
};

variadic_constructible::variadic_constructible()
    : _first(0), _second(0.0), _error_code(FT_ERR_SUCCESSS)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

variadic_constructible::variadic_constructible(int first, double second)
    : _first(first), _second(second), _error_code(FT_ERR_SUCCESSS)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

variadic_constructible::variadic_constructible(const variadic_constructible &other)
    : _first(other._first), _second(other._second), _error_code(other._error_code)
{
    this->set_error(other._error_code);
    return ;
}

variadic_constructible &variadic_constructible::operator=(const variadic_constructible &other)
{
    if (this != &other)
    {
        this->_first = other._first;
        this->_second = other._second;
        this->_error_code = other._error_code;
    }
    this->set_error(other._error_code);
    return (*this);
}

variadic_constructible::~variadic_constructible()
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void variadic_constructible::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

int variadic_constructible::first() const
{
    this->set_error(FT_ERR_SUCCESSS);
    return (this->_first);
}

double variadic_constructible::second() const
{
    this->set_error(FT_ERR_SUCCESSS);
    return (this->_second);
}

int variadic_constructible::get_error() const
{
    this->set_error(this->_error_code);
    return (this->_error_code);
}

const char *variadic_constructible::get_error_str() const
{
    this->set_error(this->_error_code);
    return (ft_strerror(this->_error_code));
}

class shared_ptr_base_type
{
    private:
        int _marker;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        shared_ptr_base_type();
        explicit shared_ptr_base_type(int marker);
        shared_ptr_base_type(const shared_ptr_base_type &other);
        shared_ptr_base_type &operator=(const shared_ptr_base_type &other);
        virtual ~shared_ptr_base_type();

        int get_marker() const;
        int get_error() const;
        const char *get_error_str() const;
};

shared_ptr_base_type::shared_ptr_base_type()
    : _marker(0), _error_code(FT_ERR_SUCCESSS)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

shared_ptr_base_type::shared_ptr_base_type(int marker)
    : _marker(marker), _error_code(FT_ERR_SUCCESSS)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

shared_ptr_base_type::shared_ptr_base_type(const shared_ptr_base_type &other)
    : _marker(other._marker), _error_code(other._error_code)
{
    this->set_error(other._error_code);
    return ;
}

shared_ptr_base_type &shared_ptr_base_type::operator=(const shared_ptr_base_type &other)
{
    if (this != &other)
    {
        this->_marker = other._marker;
        this->_error_code = other._error_code;
    }
    this->set_error(other._error_code);
    return (*this);
}

shared_ptr_base_type::~shared_ptr_base_type()
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void shared_ptr_base_type::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

int shared_ptr_base_type::get_marker() const
{
    this->set_error(FT_ERR_SUCCESSS);
    return (this->_marker);
}

int shared_ptr_base_type::get_error() const
{
    this->set_error(this->_error_code);
    return (this->_error_code);
}

const char *shared_ptr_base_type::get_error_str() const
{
    this->set_error(this->_error_code);
    return (ft_strerror(this->_error_code));
}

class shared_ptr_derived_type : public shared_ptr_base_type
{
    private:
        mutable int _error_code;

        void set_error(int error) const;

    public:
        shared_ptr_derived_type();
        explicit shared_ptr_derived_type(int marker);
        shared_ptr_derived_type(const shared_ptr_derived_type &other);
        shared_ptr_derived_type &operator=(const shared_ptr_derived_type &other);
        ~shared_ptr_derived_type();

        int get_error() const;
        const char *get_error_str() const;
};

shared_ptr_derived_type::shared_ptr_derived_type()
    : shared_ptr_base_type(), _error_code(FT_ERR_SUCCESSS)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

shared_ptr_derived_type::shared_ptr_derived_type(int marker)
    : shared_ptr_base_type(marker), _error_code(FT_ERR_SUCCESSS)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

shared_ptr_derived_type::shared_ptr_derived_type(const shared_ptr_derived_type &other)
    : shared_ptr_base_type(other), _error_code(other._error_code)
{
    this->set_error(other._error_code);
    return ;
}

shared_ptr_derived_type &shared_ptr_derived_type::operator=(const shared_ptr_derived_type &other)
{
    if (this != &other)
    {
        shared_ptr_base_type::operator=(other);
        this->_error_code = other._error_code;
    }
    this->set_error(other._error_code);
    return (*this);
}

shared_ptr_derived_type::~shared_ptr_derived_type()
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void shared_ptr_derived_type::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

int shared_ptr_derived_type::get_error() const
{
    this->set_error(this->_error_code);
    return (this->_error_code);
}

const char *shared_ptr_derived_type::get_error_str() const
{
    this->set_error(this->_error_code);
    return (ft_strerror(this->_error_code));
}

FT_TEST(test_ft_shared_ptr_variadic_constructor_uses_custom_concept, "ft_sharedptr forwards constructor arguments when custom concept is available")
{
    ft_sharedptr<variadic_constructible> pointer(21, 3.5);

    FT_ASSERT(pointer.hasError() == false);
    FT_ASSERT(pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(21, pointer->first());
    FT_ASSERT(pointer->second() > 3.4);
    FT_ASSERT(pointer->second() < 3.6);
    return (1);
}

FT_TEST(test_ft_unique_ptr_variadic_constructor_uses_custom_concept, "ft_uniqueptr forwards constructor arguments when custom concept is available")
{
    ft_uniqueptr<variadic_constructible> pointer(11, 2.5);

    FT_ASSERT(pointer.hasError() == false);
    FT_ASSERT(pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(11, pointer->first());
    FT_ASSERT(pointer->second() > 2.4);
    FT_ASSERT(pointer->second() < 2.6);
    return (1);
}

FT_TEST(test_ft_shared_ptr_convertible_constructor_uses_custom_concept, "ft_sharedptr cross-type constructors rely on custom convertible concept")
{
    ft_sharedptr<shared_ptr_derived_type> derived_pointer(new shared_ptr_derived_type(73));
    FT_ASSERT(derived_pointer.hasError() == false);

    ft_sharedptr<shared_ptr_base_type> copied_pointer(derived_pointer);
    FT_ASSERT(copied_pointer.hasError() == false);
    FT_ASSERT_EQ(2, derived_pointer.use_count());
    FT_ASSERT_EQ(2, copied_pointer.use_count());
    FT_ASSERT_EQ(73, copied_pointer->get_marker());

    ft_sharedptr<shared_ptr_base_type> moved_pointer(ft_sharedptr<shared_ptr_derived_type>(new shared_ptr_derived_type(29)));
    FT_ASSERT(moved_pointer.hasError() == false);
    FT_ASSERT(moved_pointer.get() != ft_nullptr);
    FT_ASSERT_EQ(29, moved_pointer->get_marker());
    return (1);
}

#if FT_TEMPLATE_HAS_CONCEPTS
static_assert(ft_constructible_from<variadic_constructible, int, double>);
static_assert(ft_convertible_to<shared_ptr_derived_type*, shared_ptr_base_type*>);
#endif

