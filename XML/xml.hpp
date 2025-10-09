#ifndef XML_HPP
#define XML_HPP

#include "../Template/vector.hpp"
#include "../Template/unordered_map.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"

struct xml_node
{
    char *name;
    char *text;
    ft_vector<xml_node*> children;
    ft_unordered_map<char*, char*> attributes;

    xml_node() noexcept;
    ~xml_node() noexcept;
};



class xml_document
{
    private:
        xml_node *_root;
        mutable int _error_code;

        void set_error(int error_code) const;

    public:
        xml_document() noexcept;
        ~xml_document() noexcept;

        int load_from_string(const char *xml) noexcept;
        int load_from_file(const char *file_path) noexcept;
        char *write_to_string() const noexcept;
        int write_to_file(const char *file_path) const noexcept;
        xml_node *get_root() const noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
