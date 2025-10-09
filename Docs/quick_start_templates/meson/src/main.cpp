#include "FullLibft.hpp"

int main()
{
    const char *message = "Hello from Libft";
    ft_size_t message_length = ft_strlen(message);

    pf_printf("Message length: %llu\n", (unsigned long long)message_length);

    return (0);
}
