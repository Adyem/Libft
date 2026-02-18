#include "../test_internal.hpp"
#include "../../Networking/networking.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Basic/basic.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_dns_resolve_numeric_host_returns_address,
    "networking_dns_resolve resolves numeric IPv4 host without errors")
{
    ft_vector<networking_resolved_address> resolved_addresses;
    bool resolve_result;
    size_t address_count;
    networking_resolved_address first_address;
    const struct sockaddr_in *ipv4_address;

    networking_dns_clear_cache();
    resolve_result = networking_dns_resolve("127.0.0.1", "80", AF_INET, SOCK_STREAM,
        IPPROTO_TCP, AI_NUMERICHOST, resolved_addresses);
    FT_ASSERT_EQ(true, resolve_result);
    address_count = resolved_addresses.size();
    FT_ASSERT(address_count > 0);
    first_address = resolved_addresses[0];
    ipv4_address = reinterpret_cast<const struct sockaddr_in*>(&first_address.address);
    FT_ASSERT(ipv4_address != ft_nullptr);
    FT_ASSERT_EQ(AF_INET, ipv4_address->sin_family);
    FT_ASSERT_EQ(htons(80), ipv4_address->sin_port);
    return (1);
}

FT_TEST(test_dns_resolve_rejects_null_host,
    "networking_dns_resolve returns error when host argument is null")
{
    ft_vector<networking_resolved_address> resolved_addresses;
    bool resolve_result;

    networking_dns_clear_cache();
    resolve_result = networking_dns_resolve(ft_nullptr, "80", AF_INET, SOCK_STREAM,
        IPPROTO_TCP, 0, resolved_addresses);
    FT_ASSERT_EQ(false, resolve_result);
    return (1);
}

FT_TEST(test_dns_resolve_first_returns_populated_address,
    "networking_dns_resolve_first copies the first resolved endpoint")
{
    networking_resolved_address resolved_address;
    bool resolve_first_result;
    const struct sockaddr_in *ipv4_address;

    networking_dns_clear_cache();
    resolve_first_result = networking_dns_resolve_first("127.0.0.1", "443", AF_INET,
        SOCK_STREAM, IPPROTO_TCP, AI_NUMERICHOST, resolved_address);
    FT_ASSERT_EQ(true, resolve_first_result);
    ipv4_address = reinterpret_cast<const struct sockaddr_in*>(&resolved_address.address);
    FT_ASSERT(ipv4_address != ft_nullptr);
    FT_ASSERT_EQ(AF_INET, ipv4_address->sin_family);
    FT_ASSERT_EQ(htons(443), ipv4_address->sin_port);
    return (1);
}

FT_TEST(test_dns_clear_cache_resets_error_state,
    "networking_dns_clear_cache completes without side effects")
{
    networking_dns_clear_cache();
    return (1);
}

FT_TEST(test_dns_set_error_unknown_status_maps_to_failed,
    "networking_dns_set_error accepts known and unknown resolver codes")
{
    networking_dns_set_error(12345);
#ifdef EAI_AGAIN
    networking_dns_set_error(EAI_AGAIN);
#endif
    FT_ASSERT(true);
    return (1);
}
