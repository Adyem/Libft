#include "xml.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include <cstdio>
#include <cerrno>

static const char *skip_whitespace(const char *string)
{
    while (string && (*string == ' ' || *string == '\n' || *string == '\t' || *string == '\r'))
        string++;
    return (string);
}

static char *duplicate_range(const char *start, size_t length)
{
    char *copy = static_cast<char *>(cma_malloc(length + 1));
    if (!copy)
        return (ft_nullptr);
    size_t index = 0;
    while (index < length)
    {
        copy[index] = start[index];
        index++;
    }
    copy[length] = '\0';
    return (copy);
}

xml_node::xml_node() noexcept
    : name(ft_nullptr), text(ft_nullptr), children(), attributes()
{
    return ;
}

xml_node::~xml_node() noexcept
{
    size_t child_index = 0;
    size_t child_size = this->children.size();
    while (child_index < child_size)
    {
        xml_node *child = this->children[child_index];
        if (child)
            delete child;
        child_index++;
    }
    ft_unord_map<char*, char*>::iterator it = this->attributes.begin();
    ft_unord_map<char*, char*>::iterator end = this->attributes.end();
    while (it != end)
    {
        if (it->first)
            cma_free(it->first);
        if (it->second)
            cma_free(it->second);
        ++it;
    }
    if (this->name)
        cma_free(this->name);
    if (this->text)
        cma_free(this->text);
    return ;
}

static const char *parse_node(const char *string, xml_node **out_node)
{
    string = skip_whitespace(string);
    if (!string || *string != '<')
        return (ft_nullptr);
    string++;
    const char *name_start = string;
    while (*string && *string != '>' && *string != ' ' && *string != '/')
        string++;
    size_t name_length = static_cast<size_t>(string - name_start);
    char *name = duplicate_range(name_start, name_length);
    xml_node *node = new xml_node();
    if (!node || !name)
    {
        if (node)
            delete node;
        if (name)
            cma_free(name);
        return (ft_nullptr);
    }
    node->name = name;
    while (*string && *string != '>')
        string++;
    if (*string != '>')
    {
        delete node;
        return (ft_nullptr);
    }
    string++;
    const char *text_start = string;
    if (*string == '<' && string[1] == '/');
    else if (*string == '<')
    {
        while (1)
        {
            xml_node *child = ft_nullptr;
            string = parse_node(string, &child);
            if (!string)
            {
                delete node;
                return (ft_nullptr);
            }
            node->children.push_back(child);
            string = skip_whitespace(string);
            if (*string == '<' && string[1] == '/')
                break;
        }
    }
    else
    {
        while (*string && !(*string == '<' && string[1] == '/'))
            string++;
        size_t text_length = static_cast<size_t>(string - text_start);
        node->text = duplicate_range(text_start, text_length);
    }
    if (*string != '<' || string[1] != '/')
    {
        delete node;
        return (ft_nullptr);
    }
    string += 2;
    while (*string && *string != '>')
        string++;
    if (*string != '>')
    {
        delete node;
        return (ft_nullptr);
    }
    string++;
    *out_node = node;
    return (string);
}

xml_document::xml_document() noexcept
    : _root(ft_nullptr), _error_code(ER_SUCCESS)
{
    return ;
}

xml_document::~xml_document() noexcept
{
    if (this->_root)
        delete this->_root;
    return ;
}

void xml_document::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

int xml_document::load_from_string(const char *xml) noexcept
{
    if (!xml)
    {
        this->set_error(FT_EINVAL);
        return (FT_EINVAL);
    }
    if (this->_root)
    {
        delete this->_root;
        this->_root = ft_nullptr;
    }
    xml_node *node = ft_nullptr;
    const char *end = parse_node(xml, &node);
    if (!end)
    {
        this->set_error(FT_EINVAL);
        return (FT_EINVAL);
    }
    this->_root = node;
    return (ER_SUCCESS);
}

static int read_file_content(const char *file_path, char **out_content)
{
    FILE *file = std::fopen(file_path, "rb");
    if (!file)
        return (FT_EINVAL);
    if (std::fseek(file, 0, SEEK_END) != 0)
    {
        std::fclose(file);
        return (FT_EINVAL);
    }
    long size = std::ftell(file);
    if (size <= 0)
    {
        std::fclose(file);
        return (FT_EINVAL);
    }
    if (std::fseek(file, 0, SEEK_SET) != 0)
    {
        std::fclose(file);
        return (FT_EINVAL);
    }
    char *buffer = static_cast<char *>(cma_malloc(static_cast<size_t>(size) + 1));
    if (!buffer)
    {
        std::fclose(file);
        return (CMA_BAD_ALLOC);
    }
    size_t read_size = std::fread(buffer, 1, static_cast<size_t>(size), file);
    std::fclose(file);
    if (read_size != static_cast<size_t>(size))
    {
        cma_free(buffer);
        return (FT_EINVAL);
    }
    buffer[size] = '\0';
    *out_content = buffer;
    return (ER_SUCCESS);
}

int xml_document::load_from_file(const char *file_path) noexcept
{
    char *content = ft_nullptr;
    int result = read_file_content(file_path, &content);
    if (result != ER_SUCCESS)
    {
        this->set_error(result);
        return (result);
    }
    result = this->load_from_string(content);
    cma_free(content);
    return (result);
}

static void append_string(ft_vector<char> &buffer, const char *string)
{
    if (!string)
        return ;
    size_t index = 0;
    size_t length = ft_strlen(string);
    while (index < length)
    {
        buffer.push_back(string[index]);
        index++;
    }
    return ;
}

static void write_node(const xml_node *node, ft_vector<char> &buffer)
{
    buffer.push_back('<');
    append_string(buffer, node->name);
    buffer.push_back('>');
    if (node->text)
        append_string(buffer, node->text);
    size_t index = 0;
    size_t size = node->children.size();
    while (index < size)
    {
        write_node(node->children[index], buffer);
        index++;
    }
    buffer.push_back('<');
    buffer.push_back('/');
    append_string(buffer, node->name);
    buffer.push_back('>');
    return ;
}

char *xml_document::write_to_string() const noexcept
{
    if (!this->_root)
    {
        this->set_error(FT_EINVAL);
        return (ft_nullptr);
    }
    ft_vector<char> buffer;
    write_node(this->_root, buffer);
    buffer.push_back('\n');
    buffer.push_back('\0');
    size_t length = buffer.size();
    char *result = static_cast<char *>(cma_malloc(length));
    if (!result)
    {
        this->set_error(FT_EALLOC);
        return (ft_nullptr);
    }
    size_t index = 0;
    while (index < length)
    {
        result[index] = buffer[index];
        index++;
    }
    this->set_error(ER_SUCCESS);
    return (result);
}

int xml_document::write_to_file(const char *file_path) const noexcept
{
    char *content = this->write_to_string();
    if (!content)
    {
        int error_code = this->get_error();
        if (error_code == ER_SUCCESS)
        {
            this->set_error(FT_EINVAL);
            error_code = FT_EINVAL;
        }
        return (error_code);
    }
    errno = 0;
    FILE *file = std::fopen(file_path, "wb");
    if (!file)
    {
        int error_code = errno ? errno + ERRNO_OFFSET : FT_EINVAL;
        this->set_error(error_code);
        cma_free(content);
        return (error_code);
    }
    size_t length = ft_strlen(content);
    errno = 0;
    size_t written = std::fwrite(content, 1, length, file);
    if (written != length)
    {
        int error_code = errno ? errno + ERRNO_OFFSET : FT_EINVAL;
        std::fclose(file);
        cma_free(content);
        this->set_error(error_code);
        return (error_code);
    }
    std::fclose(file);
    cma_free(content);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

xml_node *xml_document::get_root() const noexcept
{
    return (this->_root);
}

int xml_document::get_error() const noexcept
{
    return (this->_error_code);
}

const char *xml_document::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

