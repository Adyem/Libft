#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static std::string trim_whitespace(const std::string &input)
{
    std::size_t start_index = 0;
    std::size_t input_length = input.length();
    while (start_index < input_length && std::isspace(static_cast<unsigned char>(input[start_index])) != 0)
    {
        start_index += 1;
    }
    if (start_index == input_length)
    {
        return (std::string());
    }
    std::size_t end_index = input_length;
    while (end_index > start_index && std::isspace(static_cast<unsigned char>(input[end_index - 1])) != 0)
    {
        end_index -= 1;
    }
    return (input.substr(start_index, end_index - start_index));
}

static std::vector<std::string> read_includes(const std::string &path, const std::string &prefix)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
    {
        std::cerr << "Failed to open " << path << '\n';
        return (std::vector<std::string>());
    }
    std::vector<std::string> includes;
    std::string line_content;
    while (std::getline(file, line_content))
    {
        if (line_content.rfind(prefix, 0) == 0)
        {
            std::size_t begin = prefix.length();
            std::size_t end_position = line_content.find('"', begin);
            if (end_position == std::string::npos)
            {
                std::cerr << "Malformed include in " << path << ": " << line_content << '\n';
                return (std::vector<std::string>());
            }
            includes.push_back(line_content.substr(begin, end_position - begin));
        }
    }
    return (includes);
}

static std::vector<std::string> read_manifest(const std::string &path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
    {
        std::cerr << "Failed to open " << path << '\n';
        return (std::vector<std::string>());
    }
    std::vector<std::string> entries;
    std::string line_content;
    while (std::getline(file, line_content))
    {
        std::string trimmed = trim_whitespace(line_content);
        if (!trimmed.empty() && trimmed[0] != '#')
        {
            entries.push_back(trimmed);
        }
    }
    return (entries);
}

int main()
{
    const std::string header_path = "Libft/FullLibft.hpp";
    const std::string manifest_path = "Libft/full_libft_manifest.txt";
    std::vector<std::string> header_includes = read_includes(header_path, "#include \"");
    if (header_includes.empty())
    {
        std::cerr << "No includes found in " << header_path << '\n';
        return (1);
    }
    std::vector<std::string> manifest_entries = read_manifest(manifest_path);
    if (manifest_entries.empty())
    {
        std::cerr << "Manifest is empty: " << manifest_path << '\n';
        return (1);
    }
    std::vector<std::string> sorted_header = header_includes;
    std::sort(sorted_header.begin(), sorted_header.end());
    if (sorted_header != header_includes)
    {
        std::cerr << "Header includes are not sorted alphabetically." << '\n';
        return (1);
    }
    std::vector<std::string> sorted_manifest = manifest_entries;
    std::sort(sorted_manifest.begin(), sorted_manifest.end());
    if (sorted_manifest != manifest_entries)
    {
        std::cerr << "Manifest entries are not sorted alphabetically." << '\n';
        return (1);
    }
    if (header_includes != manifest_entries)
    {
        std::cerr << "Header includes and manifest entries differ." << '\n';
        return (1);
    }
    std::size_t index = 1;
    std::size_t header_count = header_includes.size();
    while (index < header_count)
    {
        if (header_includes[index] == header_includes[index - 1])
        {
            std::cerr << "Duplicate include detected: " << header_includes[index] << '\n';
            return (1);
        }
        index += 1;
    }
    return (0);
}
