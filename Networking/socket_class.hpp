#ifndef FT_SOCKET_H
#define FT_SOCKET_H

#include "networking.hpp"
#include "../Template/vector.hpp"
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# define FT_CLOSE_SOCKET(fd) closesocket(fd)
#else
# include <sys/socket.h>
# include <sys/types.h>
# include <unistd.h>
# define FT_CLOSE_SOCKET(fd) close(fd)
#endif

int nw_bind(ssize_t sockfd, const struct sockaddr *addr, socklen_t addrlen);
int nw_listen(ssize_t sockfd, int backlog);
int nw_accept(ssize_t sockfd, struct sockaddr *addr, socklen_t *addrlen);
int nw_socket(int domain, int type, int protocol);
int nw_connect(ssize_t sockfd, const struct sockaddr *addr, socklen_t addrlen);

class ft_socket
{
	private:
		int 	setup_server(const SocketConfig &config);
    	int 	setup_client(const SocketConfig &config);
		int 	create_socket(const SocketConfig &config);
    	int 	set_reuse_address(const SocketConfig &config);
    	int 	set_non_blocking(const SocketConfig &config);
    	int 	set_timeouts(const SocketConfig &config);
    	int 	configure_address(const SocketConfig &config);
    	int 	bind_socket(const SocketConfig &config);
    	int 	listen_socket(const SocketConfig &config);
		int		accept_connection();
    	void 	handle_error(int error_code);

		struct sockaddr_storage _address;
		ft_vector<ft_socket> 	_connected;
    	int		 				_socket_fd;
    	int 					_error;

		ft_socket(int fd, const sockaddr_storage &addr);
		ft_socket(const ft_socket &other) = delete;
    	ft_socket &operator=(const ft_socket &other) = delete;

	public:
		ft_socket(const SocketConfig &config);
    	ft_socket();
    	~ft_socket();

		ft_socket(ft_socket &&other) noexcept;
		ft_socket &operator=(ft_socket &&other) noexcept;

		int			initialize(const SocketConfig &config);
    	ssize_t 	send_data(const void *data, size_t size, int flags = 0);
    	ssize_t		receive_data(void *buffer, size_t size, int flags = 0);
    	bool		close_socket();
    	int 		get_error() const;
    	const char	*get_error_message() const;
		ssize_t 	broadcast_data(const void *data, size_t size, int flags);
		ssize_t 	broadcast_data(const void *data, size_t size, int flags, int exception);
		ssize_t 	send_data(const void *data, size_t size, int flags, int fd);
		int			get_fd() const;
};

#endif
