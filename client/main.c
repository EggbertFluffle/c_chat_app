#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

const int MESSAGE_BUF_LEN = 512;

int main(void) {
	// List of sockets to
	struct pollfd pfds[2];
	memset((void*)pfds, 0, sizeof(struct pollfd) * 2);

	pfds[0].fd = 0;
	pfds[0].events = POLLIN;

	pfds[1].fd = socket(AF_INET, SOCK_STREAM, 0);
	pfds[1].events = POLLIN;
	if(pfds[1].fd == -1) {
		puts("Unable to create a socket!");
		return -1;
	}

	char* ipv4_address = (char*) calloc(64, sizeof(char));
	int res_len = printf("What ip would you like to connect to?: ");
	if(res_len != 0) scanf(" %s", ipv4_address);

	int32_t port = 25565;
	res_len = printf("What port would you like to connect through?: ");
	if(res_len != 0) scanf(" %d", &port);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; // use ipv4
	addr.sin_port = htons(25565); // put that jawn in network byte order
	addr.sin_addr.s_addr = inet_addr(ipv4_address); // local host
	
	if(connect(pfds[1].fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error code: %d\n", errno);
		perror("Connection to host failed!");
		close(pfds[1].fd);
		return -1;
	}

	{
		int opts = fcntl(pfds[0].fd, F_GETFL, 0);
		fcntl(pfds[0].fd, F_SETFL, opts | O_NONBLOCK);

		opts = fcntl(pfds[1].fd, F_GETFL, 0);
		fcntl(pfds[1].fd, F_SETFL, opts | O_NONBLOCK);
	}

	void* message_buf = calloc(MESSAGE_BUF_LEN, sizeof(char));
	for(;;) {
		if(poll(pfds, 2, -1) == -1) {
			printf("Poll failed with error code %d\n", errno);
			continue;
		}
		
		if(pfds[0].revents & POLLIN) {
			size_t message_len = read(pfds[0].fd, message_buf, MESSAGE_BUF_LEN);

			if(message_len != -1) {
				if(write(pfds[1].fd, message_buf, message_len) == -1) {
					perror("Message writing failed!\n");
				}
			}
		}

		if(pfds[1].revents & POLLIN) {
			size_t message_len = read(pfds[1].fd, message_buf, MESSAGE_BUF_LEN);
			if(message_len != -1) {
				// I don't think I need a pfds for this
				write(1, message_buf, message_len);
			}
		}
	}
	
	close(pfds[0].fd);
	close(pfds[1].fd);
	return 0;
}
