#ifndef API_REQUEST_SIGNING_HPP
#define API_REQUEST_SIGNING_HPP

#include <cstddef>
#include "../CPP_class/class_string_class.hpp"

struct api_hmac_signature_input
{
    const char *method;
    const char *path;
    const char *canonical_headers;
    const char *canonical_query;
    const char *body;
};

int api_sign_request_hmac_sha256(const api_hmac_signature_input &input,
    const unsigned char *key, std::size_t key_length,
    ft_string &signature_output) noexcept;

int api_apply_hmac_signature_header(const api_hmac_signature_input &input,
    const unsigned char *key, std::size_t key_length,
    const char *header_name, ft_string &header_output) noexcept;

struct api_oauth1_parameter
{
    const char *key;
    const char *value;
};

struct api_oauth1_parameters
{
    const char *method;
    const char *url;
    const char *consumer_key;
    const char *consumer_secret;
    const char *token;
    const char *token_secret;
    const char *timestamp;
    const char *nonce;
    const char *signature_method;
    const char *version;
    const api_oauth1_parameter *additional_parameters;
    std::size_t additional_parameter_count;
};

int api_build_oauth1_authorization_header(
    const api_oauth1_parameters &parameters, ft_string &header_output) noexcept;

#endif
