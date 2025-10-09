#include "xml.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include <cstdio>
#include <cerrno>
#include <new>

static const char *skip_whitespace(const char *string)
{
    while (string && (*string == ' ' || *string == '\n' || *string == '\t' || *string == '\r'))
        string++;
    return (string);
}

static int  is_xml_whitespace(char character)
{
    if (character == ' ')
        return (1);
    if (character == '\n')
        return (1);
    if (character == '\t')
        return (1);
    if (character == '\r')
        return (1);
    return (0);
}

static char *duplicate_range(const char *start, size_t length)
{
    char *copy = static_cast<char *>(cma_malloc(length + 1));
    if (!copy)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    size_t index = 0;
    while (index < length)
    {
        copy[index] = start[index];
        index++;
    }
    copy[length] = '\0';
    return (copy);
}

static int translate_vector_error(int error_code)
{
    if (error_code == ER_SUCCESS)
        return (ER_SUCCESS);
    if (error_code == FT_ERR_NO_MEMORY)
        return (FT_ERR_NO_MEMORY);
    return (error_code);
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
    ft_unordered_map<char*, char*>::iterator it = this->attributes.begin();
    ft_unordered_map<char*, char*>::iterator end = this->attributes.end();
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
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    string++;
    const char *name_start = string;
    while (*string && *string != '>' && *string != ' ' && *string != '/')
        string++;
    size_t name_length = static_cast<size_t>(string - name_start);
    char *name = duplicate_range(name_start, name_length);
    if (!name)
        return (ft_nullptr);
    xml_node *node = new (std::nothrow) xml_node();
    if (!node)
    {
        cma_free(name);
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    node->name = name;
    char quote_character;

    quote_character = '\0';
    while (*string)
    {
        if (*string == '>' && quote_character == '\0')
            break ;
        if (*string == '"' || *string == '\'')
        {
            if (quote_character == '\0')
                quote_character = *string;
            else if (quote_character == *string)
                quote_character = '\0';
        }
        string++;
    }
    if (*string != '>')
    {
        delete node;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    const char *tag_end = string;
    int self_closing = 0;
    if (tag_end > name_start)
    {
        const char *before_close = tag_end - 1;
        while (before_close >= name_start)
        {
            if (*before_close == '/')
            {
                self_closing = 1;
                break;
            }
            if (*before_close != ' ' && *before_close != '\n' && *before_close != '\t' && *before_close != '\r')
                break;
            before_close--;
        }
    }
    string++;
    if (self_closing)
    {
        *out_node = node;
        ft_errno = ER_SUCCESS;
        return (string);
    }
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
            int children_error_code = node->children.get_error();
            if (children_error_code != ER_SUCCESS)
            {
                delete child;
                delete node;
                ft_errno = translate_vector_error(children_error_code);
                return (ft_nullptr);
            }
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
        if (!node->text && text_length > 0)
        {
            delete node;
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
    }
    if (*string != '<' || string[1] != '/')
    {
        delete node;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    string += 2;
    string = skip_whitespace(string);
    const char *closing_name_start = string;
    while (*string && *string != '>' && !is_xml_whitespace(*string))
        string++;
    size_t closing_name_length = static_cast<size_t>(string - closing_name_start);
    size_t opening_name_length = ft_strlen(node->name);
    if (closing_name_length != opening_name_length)
    {
        delete node;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    size_t name_index = 0;
    while (name_index < closing_name_length)
    {
        if (closing_name_start[name_index] != node->name[name_index])
        {
            delete node;
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        name_index++;
    }
    while (*string && *string != '>')
    {
        if (!is_xml_whitespace(*string))
        {
            delete node;
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        string++;
    }
    if (*string != '>')
    {
        delete node;
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    string++;
    *out_node = node;
    ft_errno = ER_SUCCESS;
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
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
        int error_code = ft_errno ? ft_errno : FT_ERR_INVALID_ARGUMENT;
        this->set_error(error_code);
        return (error_code);
    }
    const char *remaining = skip_whitespace(end);
    if (remaining && *remaining != '\0')
    {
        delete node;
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_root = node;
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

static int read_file_content(const char *file_path, char **out_content)
{
    errno = 0;
    FILE *file = std::fopen(file_path, "rb");
    if (!file)
    {
        int error_code = errno ? errno + ERRNO_OFFSET : FT_ERR_INVALID_ARGUMENT;
        ft_errno = error_code;
        return (error_code);
    }
    errno = 0;
    if (std::fseek(file, 0, SEEK_END) != 0)
    {
        int error_code = errno ? errno + ERRNO_OFFSET : FT_ERR_INVALID_ARGUMENT;
        std::fclose(file);
        ft_errno = error_code;
        return (error_code);
    }
    long size = std::ftell(file);
    if (size <= 0)
    {
        std::fclose(file);
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    errno = 0;
    if (std::fseek(file, 0, SEEK_SET) != 0)
    {
        int error_code = errno ? errno + ERRNO_OFFSET : FT_ERR_INVALID_ARGUMENT;
        std::fclose(file);
        ft_errno = error_code;
        return (error_code);
    }
    char *buffer = static_cast<char *>(cma_malloc(static_cast<size_t>(size) + 1));
    if (!buffer)
    {
        std::fclose(file);
        ft_errno = FT_ERR_NO_MEMORY;
        return (FT_ERR_NO_MEMORY);
    }
    errno = 0;
    size_t read_size = std::fread(buffer, 1, static_cast<size_t>(size), file);
    std::fclose(file);
    if (read_size != static_cast<size_t>(size))
    {
        cma_free(buffer);
        int error_code = errno ? errno + ERRNO_OFFSET : FT_ERR_INVALID_ARGUMENT;
        ft_errno = error_code;
        return (error_code);
    }
    buffer[size] = '\0';
    *out_content = buffer;
    ft_errno = ER_SUCCESS;
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
    if (result == ER_SUCCESS)
        this->set_error(ER_SUCCESS);
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    char *result = ft_nullptr;
    int error_code = ER_SUCCESS;
    {
        ft_vector<char> buffer;
        write_node(this->_root, buffer);
        int buffer_error = buffer.get_error();
        if (buffer_error != ER_SUCCESS)
            error_code = translate_vector_error(buffer_error);
        if (error_code == ER_SUCCESS)
        {
            buffer.push_back('\n');
            buffer_error = buffer.get_error();
            if (buffer_error != ER_SUCCESS)
                error_code = translate_vector_error(buffer_error);
        }
        if (error_code == ER_SUCCESS)
        {
            buffer.push_back('\0');
            buffer_error = buffer.get_error();
            if (buffer_error != ER_SUCCESS)
                error_code = translate_vector_error(buffer_error);
        }
        if (error_code == ER_SUCCESS)
        {
            size_t length = buffer.size();
            result = static_cast<char *>(cma_malloc(length));
            if (!result)
                error_code = FT_ERR_NO_MEMORY;
            else
            {
                size_t index = 0;
                while (index < length)
                {
                    result[index] = buffer[index];
                    index++;
                }
            }
        }
    }
    if (error_code != ER_SUCCESS)
    {
        if (result)
            cma_free(result);
        this->set_error(error_code);
        return (ft_nullptr);
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
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            error_code = FT_ERR_INVALID_ARGUMENT;
        }
        return (error_code);
    }
    errno = 0;
    FILE *file = std::fopen(file_path, "wb");
    if (!file)
    {
        int error_code = errno ? errno + ERRNO_OFFSET : FT_ERR_INVALID_ARGUMENT;
        cma_free(content);
        this->set_error(error_code);
        return (error_code);
    }
    size_t length = ft_strlen(content);
    errno = 0;
    size_t written = std::fwrite(content, 1, length, file);
    if (written != length)
    {
        int error_code = errno ? errno + ERRNO_OFFSET : FT_ERR_INVALID_ARGUMENT;
        std::fclose(file);
        cma_free(content);
        this->set_error(error_code);
        return (error_code);
    }
    errno = 0;
    if (std::fclose(file) != 0)
    {
        int error_code = errno ? errno + ERRNO_OFFSET : FT_ERR_INVALID_ARGUMENT;
        cma_free(content);
        this->set_error(error_code);
        return (error_code);
    }
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

