#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "../../System_utils/test_runner.hpp"

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

static std::string resolve_existing_path(const std::vector<std::string> &candidates)
{
    std::size_t index = 0;
    std::size_t candidate_count = candidates.size();
    while (index < candidate_count)
    {
        std::ifstream file(candidates[index].c_str());
        if (file.is_open())
        {
            return (candidates[index]);
        }
        index += 1;
    }
    return (std::string());
}

FT_TEST(test_full_libft_header_manifest_sync, "full libft header matches manifest")
{
    std::vector<std::string> header_candidates;
    std::vector<std::string> manifest_candidates;
    header_candidates.push_back("FullLibft.hpp");
    header_candidates.push_back("../FullLibft.hpp");
    header_candidates.push_back("Libft/FullLibft.hpp");
    manifest_candidates.push_back("full_libft_manifest.txt");
    manifest_candidates.push_back("../full_libft_manifest.txt");
    manifest_candidates.push_back("Libft/full_libft_manifest.txt");
    std::string header_path = resolve_existing_path(header_candidates);
    std::string manifest_path = resolve_existing_path(manifest_candidates);
    FT_ASSERT(header_path.empty() == false);
    FT_ASSERT(manifest_path.empty() == false);
    std::vector<std::string> header_includes = read_includes(header_path, "#include \"");
    FT_ASSERT(header_includes.empty() == false);
    std::vector<std::string> manifest_entries = read_manifest(manifest_path);
    FT_ASSERT(manifest_entries.empty() == false);
    std::vector<std::string> sorted_header = header_includes;
    std::sort(sorted_header.begin(), sorted_header.end());
    FT_ASSERT(sorted_header == header_includes);
    std::vector<std::string> sorted_manifest = manifest_entries;
    std::sort(sorted_manifest.begin(), sorted_manifest.end());
    FT_ASSERT(sorted_manifest == manifest_entries);
    FT_ASSERT(header_includes == manifest_entries);
    std::size_t index = 1;
    std::size_t header_count = header_includes.size();
    while (index < header_count)
    {
        FT_ASSERT(header_includes[index] != header_includes[index - 1]);
        index += 1;
    }
    return (1);
}
