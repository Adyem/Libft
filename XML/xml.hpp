#ifndef XML_HPP
#define XML_HPP

#include "../Template/vector.hpp"
#include "../Template/unordened_map.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"

struct xml_node
{
    char *name;
    char *text;
    ft_vector<xml_node*> children;
    ft_unord_map<char*, char*> attributes;

    xml_node() noexcept;
    ~xml_node() noexcept;
};

/*
Example usage:
xml_document doc;
doc.load_from_string("<root><child>value</child></root>");
xml_node *root = doc.get_root();
char *output = doc.write_to_string();
if (output)
    cma_free(output);
*/

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
