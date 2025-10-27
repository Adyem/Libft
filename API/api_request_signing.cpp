#include "api_request_signing.hpp"
#include "../Compression/compression.hpp"
#include "../Encryption/encryption_hmac_sha256.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include <algorithm>
#include <vector>
#include <cstring>

namespace
{
    void api_request_signing_set_error(int error_code) noexcept
    {
        ft_errno = error_code;
        return ;
    }

    int api_request_signing_append(ft_string &target,
        const char *value) noexcept
    {
        if (!value)
            return (0);
        target += value;
        if (target.get_error() != ER_SUCCESS)
        {
            api_request_signing_set_error(target.get_error());
            return (-1);
        }
        return (0);
    }

    int api_request_signing_append_character(ft_string &target,
        char value) noexcept
    {
        target.append(value);
        if (target.get_error() != ER_SUCCESS)
        {
            api_request_signing_set_error(target.get_error());
            return (-1);
        }
        return (0);
    }

    int api_request_signing_percent_encode(const char *input,
        ft_string &output) noexcept
    {
        static const char hex_table[] = "0123456789ABCDEF";
        size_t index;

        output.clear();
        if (!input)
            return (0);
        index = 0;
        while (input[index] != '\0')
        {
            unsigned char character;
            bool unreserved;

            character = static_cast<unsigned char>(input[index]);
            unreserved = false;
            if (character >= 'A' && character <= 'Z')
                unreserved = true;
            else if (character >= 'a' && character <= 'z')
                unreserved = true;
            else if (character >= '0' && character <= '9')
                unreserved = true;
            else if (character == '-')
                unreserved = true;
            else if (character == '.')
                unreserved = true;
            else if (character == '_')
                unreserved = true;
            else if (character == '~')
                unreserved = true;
            if (unreserved)
            {
                output.append(static_cast<char>(character));
                if (output.get_error() != ER_SUCCESS)
                {
                    api_request_signing_set_error(output.get_error());
                    return (-1);
                }
            }
            else
            {
                char encoded_chunk[4];

                encoded_chunk[0] = '%';
                encoded_chunk[1] = hex_table[(character >> 4) & 0x0F];
                encoded_chunk[2] = hex_table[character & 0x0F];
                encoded_chunk[3] = '\0';
                output.append(encoded_chunk, 3);
                if (output.get_error() != ER_SUCCESS)
                {
                    api_request_signing_set_error(output.get_error());
                    return (-1);
                }
            }
            index += 1;
        }
        return (0);
    }

    int api_request_signing_uppercase(const char *input,
        ft_string &output) noexcept
    {
        size_t index;

        if (!input)
        {
            api_request_signing_set_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        output.clear();
        index = 0;
        while (input[index] != '\0')
        {
            unsigned char character;

            character = static_cast<unsigned char>(input[index]);
            if (character >= 'a' && character <= 'z')
                character = static_cast<unsigned char>(character - 'a' + 'A');
            output.append(static_cast<char>(character));
            if (output.get_error() != ER_SUCCESS)
            {
                api_request_signing_set_error(output.get_error());
                return (-1);
            }
            index += 1;
        }
        return (0);
    }

    int api_request_signing_build_canonical(
        const api_hmac_signature_input &input, ft_string &canonical) noexcept
    {
        if (!input.method || !input.path)
        {
            api_request_signing_set_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        canonical.clear();
        if (api_request_signing_append(canonical, input.method) != 0)
            return (-1);
        if (api_request_signing_append_character(canonical, '\n') != 0)
            return (-1);
        if (api_request_signing_append(canonical, input.path) != 0)
            return (-1);
        if (input.canonical_headers && input.canonical_headers[0] != '\0')
        {
            if (api_request_signing_append_character(canonical, '\n') != 0)
                return (-1);
            if (api_request_signing_append(canonical,
                    input.canonical_headers) != 0)
                return (-1);
        }
        if (input.canonical_query && input.canonical_query[0] != '\0')
        {
            if (api_request_signing_append_character(canonical, '\n') != 0)
                return (-1);
            if (api_request_signing_append(canonical,
                    input.canonical_query) != 0)
                return (-1);
        }
        if (input.body && input.body[0] != '\0')
        {
            if (api_request_signing_append_character(canonical, '\n') != 0)
                return (-1);
            if (api_request_signing_append(canonical, input.body) != 0)
                return (-1);
        }
        api_request_signing_set_error(ER_SUCCESS);
        return (0);
    }

    struct api_oauth_parameter_entry
    {
        ft_string key;
        ft_string value;
    };

    int api_request_signing_add_parameter(
        std::vector<api_oauth_parameter_entry> &entries,
        const char *key, const char *value) noexcept
    {
        api_oauth_parameter_entry entry;
        const char *local_value;

        if (!key)
        {
            api_request_signing_set_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        local_value = value;
        if (!local_value)
            local_value = "";
        if (api_request_signing_percent_encode(key, entry.key) != 0)
            return (-1);
        if (api_request_signing_percent_encode(local_value, entry.value) != 0)
            return (-1);
        entries.push_back(entry);
        return (0);
    }

    int api_request_signing_append_normalized(
        ft_string &target,
        const std::vector<api_oauth_parameter_entry> &entries) noexcept
    {
        size_t index;

        index = 0;
        while (index < entries.size())
        {
            const api_oauth_parameter_entry &entry = entries[index];

            if (index > 0)
            {
                if (api_request_signing_append_character(target, '&') != 0)
                    return (-1);
            }
            if (api_request_signing_append(target, entry.key.c_str()) != 0)
                return (-1);
            if (api_request_signing_append_character(target, '=') != 0)
                return (-1);
            if (api_request_signing_append(target, entry.value.c_str()) != 0)
                return (-1);
            index += 1;
        }
        return (0);
    }

    int api_request_signing_append_header_parameter(ft_string &target,
        const char *key, const char *value, int &is_first) noexcept
    {
        ft_string encoded_value;
        const char *local_value;

        if (!key)
        {
            api_request_signing_set_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        local_value = value;
        if (!local_value)
            local_value = "";
        if (api_request_signing_percent_encode(local_value,
                encoded_value) != 0)
            return (-1);
        if (!encoded_value.c_str())
            return (-1);
        if (!is_first)
        {
            if (api_request_signing_append(target, ", ") != 0)
                return (-1);
        }
        else
            is_first = 0;
        if (api_request_signing_append(target, key) != 0)
            return (-1);
        if (api_request_signing_append(target, "=\"") != 0)
            return (-1);
        if (api_request_signing_append(target, encoded_value.c_str()) != 0)
            return (-1);
        if (api_request_signing_append_character(target, '"') != 0)
            return (-1);
        return (0);
    }
}

int api_sign_request_hmac_sha256(const api_hmac_signature_input &input,
    const unsigned char *key, std::size_t key_length,
    ft_string &signature_output) noexcept
{
    unsigned char digest[32];
    ft_string canonical;
    std::size_t encoded_size;
    unsigned char *encoded_buffer;

    if (!key || key_length == 0)
    {
        api_request_signing_set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (api_request_signing_build_canonical(input, canonical) != 0)
        return (-1);
    hmac_sha256(key, key_length, canonical.c_str(), canonical.size(), digest);
    if (ft_errno != ER_SUCCESS)
        return (-1);
    encoded_buffer = ft_base64_encode(digest, sizeof(digest), &encoded_size);
    if (!encoded_buffer)
        return (-1);
    signature_output = reinterpret_cast<const char *>(encoded_buffer);
    cma_free(encoded_buffer);
    if (signature_output.get_error() != ER_SUCCESS)
    {
        api_request_signing_set_error(signature_output.get_error());
        return (-1);
    }
    api_request_signing_set_error(ER_SUCCESS);
    return (0);
}

int api_apply_hmac_signature_header(const api_hmac_signature_input &input,
    const unsigned char *key, std::size_t key_length,
    const char *header_name, ft_string &header_output) noexcept
{
    ft_string signature;

    if (!header_name)
    {
        api_request_signing_set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (api_sign_request_hmac_sha256(input, key, key_length,
            signature) != 0)
        return (-1);
    header_output.clear();
    if (api_request_signing_append(header_output, header_name) != 0)
        return (-1);
    if (api_request_signing_append(header_output, ": ") != 0)
        return (-1);
    if (api_request_signing_append(header_output, signature.c_str()) != 0)
        return (-1);
    api_request_signing_set_error(ER_SUCCESS);
    return (0);
}

int api_build_oauth1_authorization_header(
    const api_oauth1_parameters &parameters, ft_string &header_output) noexcept
{
    std::vector<api_oauth_parameter_entry> entries;
    ft_string normalized_parameters;
    ft_string method_upper;
    ft_string encoded_method;
    ft_string encoded_url;
    ft_string encoded_params;
    ft_string signing_key;
    ft_string encoded_consumer_secret;
    ft_string encoded_token_secret;
    unsigned char digest[32];
    std::size_t encoded_size;
    unsigned char *encoded_buffer;
    ft_string signature_string;
    ft_string encoded_signature;
    int header_first;
    size_t parameter_index;

    if (!parameters.method || !parameters.url
        || !parameters.consumer_key || !parameters.consumer_secret
        || !parameters.timestamp || !parameters.nonce)
    {
        api_request_signing_set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (api_request_signing_add_parameter(entries, "oauth_consumer_key",
            parameters.consumer_key) != 0)
        return (-1);
    if (api_request_signing_add_parameter(entries, "oauth_nonce",
            parameters.nonce) != 0)
        return (-1);
    if (api_request_signing_add_parameter(entries,
            "oauth_signature_method",
            parameters.signature_method ? parameters.signature_method
                : "HMAC-SHA256") != 0)
        return (-1);
    if (api_request_signing_add_parameter(entries, "oauth_timestamp",
            parameters.timestamp) != 0)
        return (-1);
    if (parameters.token)
    {
        if (api_request_signing_add_parameter(entries, "oauth_token",
                parameters.token) != 0)
            return (-1);
    }
    if (api_request_signing_add_parameter(entries, "oauth_version",
            parameters.version ? parameters.version : "1.0") != 0)
        return (-1);
    parameter_index = 0;
    while (parameter_index < parameters.additional_parameter_count)
    {
        const api_oauth1_parameter &extra =
            parameters.additional_parameters[parameter_index];

        if (api_request_signing_add_parameter(entries, extra.key,
                extra.value) != 0)
            return (-1);
        parameter_index += 1;
    }
    std::sort(entries.begin(), entries.end(),
        [](const api_oauth_parameter_entry &lhs,
            const api_oauth_parameter_entry &rhs)
        {
            int comparison;

            comparison = std::strcmp(lhs.key.c_str(), rhs.key.c_str());
            if (comparison < 0)
                return (true);
            if (comparison > 0)
                return (false);
            return (std::strcmp(lhs.value.c_str(), rhs.value.c_str()) < 0);
        });
    normalized_parameters.clear();
    if (api_request_signing_append_normalized(normalized_parameters,
            entries) != 0)
        return (-1);
    if (api_request_signing_uppercase(parameters.method, method_upper) != 0)
        return (-1);
    if (api_request_signing_percent_encode(method_upper.c_str(),
            encoded_method) != 0)
        return (-1);
    if (api_request_signing_percent_encode(parameters.url,
            encoded_url) != 0)
        return (-1);
    if (api_request_signing_percent_encode(normalized_parameters.c_str(),
            encoded_params) != 0)
        return (-1);
    ft_string base_string;

    base_string = encoded_method.c_str();
    if (base_string.get_error() != ER_SUCCESS)
    {
        api_request_signing_set_error(base_string.get_error());
        return (-1);
    }
    if (api_request_signing_append(base_string, "&") != 0)
        return (-1);
    if (api_request_signing_append(base_string, encoded_url.c_str()) != 0)
        return (-1);
    if (api_request_signing_append(base_string, "&") != 0)
        return (-1);
    if (api_request_signing_append(base_string, encoded_params.c_str()) != 0)
        return (-1);
    if (api_request_signing_percent_encode(parameters.consumer_secret,
            encoded_consumer_secret) != 0)
        return (-1);
    if (api_request_signing_percent_encode(parameters.token_secret ?
            parameters.token_secret : "", encoded_token_secret) != 0)
        return (-1);
    signing_key = encoded_consumer_secret.c_str();
    if (signing_key.get_error() != ER_SUCCESS)
    {
        api_request_signing_set_error(signing_key.get_error());
        return (-1);
    }
    if (api_request_signing_append(signing_key, "&") != 0)
        return (-1);
    if (api_request_signing_append(signing_key,
            encoded_token_secret.c_str()) != 0)
        return (-1);
    hmac_sha256(reinterpret_cast<const unsigned char *>(signing_key.c_str()),
        signing_key.size(), base_string.c_str(), base_string.size(), digest);
    if (ft_errno != ER_SUCCESS)
        return (-1);
    encoded_buffer = ft_base64_encode(digest, sizeof(digest), &encoded_size);
    if (!encoded_buffer)
        return (-1);
    signature_string = reinterpret_cast<const char *>(encoded_buffer);
    cma_free(encoded_buffer);
    if (signature_string.get_error() != ER_SUCCESS)
    {
        api_request_signing_set_error(signature_string.get_error());
        return (-1);
    }
    if (api_request_signing_percent_encode(signature_string.c_str(),
            encoded_signature) != 0)
        return (-1);
    header_output.clear();
    if (api_request_signing_append(header_output,
            "Authorization: OAuth ") != 0)
        return (-1);
    header_first = 1;
    if (api_request_signing_append_header_parameter(header_output,
            "oauth_consumer_key", parameters.consumer_key,
            header_first) != 0)
        return (-1);
    if (api_request_signing_append_header_parameter(header_output,
            "oauth_nonce", parameters.nonce, header_first) != 0)
        return (-1);
    if (api_request_signing_append_header_parameter(header_output,
            "oauth_signature_method",
            parameters.signature_method ? parameters.signature_method
                : "HMAC-SHA256", header_first) != 0)
        return (-1);
    if (api_request_signing_append_header_parameter(header_output,
            "oauth_timestamp", parameters.timestamp, header_first) != 0)
        return (-1);
    if (parameters.token)
    {
        if (api_request_signing_append_header_parameter(header_output,
                "oauth_token", parameters.token, header_first) != 0)
            return (-1);
    }
    if (api_request_signing_append_header_parameter(header_output,
            "oauth_version", parameters.version ? parameters.version : "1.0",
            header_first) != 0)
        return (-1);
    parameter_index = 0;
    while (parameter_index < parameters.additional_parameter_count)
    {
        const api_oauth1_parameter &extra =
            parameters.additional_parameters[parameter_index];

        if (api_request_signing_append_header_parameter(header_output,
                extra.key, extra.value, header_first) != 0)
            return (-1);
        parameter_index += 1;
    }
    if (api_request_signing_append(header_output, ", ") != 0)
        return (-1);
    if (api_request_signing_append(header_output,
            "oauth_signature=\"") != 0)
        return (-1);
    if (api_request_signing_append(header_output,
            encoded_signature.c_str()) != 0)
        return (-1);
    if (api_request_signing_append_character(header_output, '"') != 0)
        return (-1);
    api_request_signing_set_error(ER_SUCCESS);
    return (0);
}
