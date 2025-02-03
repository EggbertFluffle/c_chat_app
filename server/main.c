#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

const int MAX_CLIENTS = 64;
const int CONNECTION_BACKLOG = 32;
const int DEFAULT_PORT = 25565;
const int READ_BUFFER_SIZE = 512;

void accept_client(struct pollfd* pfds, int* pfds_count);
void remove_client(int* client_fds, int fd);
int get_next_client(struct pollfd* pfds, int pfds_count);
void propogate_message(void* message_buf, int message_len, struct pollfd* pfds, int pfds_count, int from_fd);

int main(void) {
	int pfds_count = 0;
	struct pollfd pfds[MAX_CLIENTS];
	memset(pfds, 0, sizeof(struct pollfd) * MAX_CLIENTS);

	// PF_INET and AF_INET are equal but this is best practice or smth
	pfds[0].fd = socket(PF_INET, SOCK_STREAM, 0);
	pfds[0].events = POLLIN;
	if(pfds[0].fd == -1) {
		puts("Unable to create a socket!");
		return -1;
	}
	pfds_count++;

	int opts = fcntl(pfds[0].fd, F_GETFD, 0);
	fcntl(pfds[0].fd, F_SETFD, opts | O_NONBLOCK);


	char* ipv4_address = (char*) calloc(64, sizeof(char));
	int res_len = printf("What ip would you like to bind to?: ");
	if(res_len != 0) scanf(" %s", ipv4_address);

	int32_t port = DEFAULT_PORT;
	res_len = printf("What port would you like to allow through?: ");
	if(res_len != 0) scanf(" %d", &port);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; // use ipv4
	addr.sin_port = htons(25565); // put that jawn in network byte order
	addr.sin_addr.s_addr = inet_addr(ipv4_address); // local host
	
	if(bind(pfds[0].fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("Bind failed!");
		close(pfds[0].fd);
		return -1;
	}

	printf("Sucessfully bound server to %s on port %d!\n", ipv4_address, port);

	if(listen(pfds[0].fd, CONNECTION_BACKLOG) != 0) {
		perror("Error trying to start listening for connections!");
		close(pfds[0].fd);
		return -1;
	}
	printf("listening...\n");

	void* message_buf = calloc(READ_BUFFER_SIZE, sizeof(char));
	for(;;) {
		if(poll(pfds, pfds_count, -1) == -1) {
			printf("Poll failed with error code %d\n", errno);
		}

		if(pfds[0].revents & POLLIN) {
			accept_client(pfds, &pfds_count);
			printf("There are %d clients connected\n", pfds_count - 1);
		}

		int inc_fd;
		while((inc_fd = get_next_client(pfds, pfds_count)) != -1) {
			int message_len = read(inc_fd, message_buf, READ_BUFFER_SIZE);
			propogate_message(message_buf, message_len, pfds, pfds_count, inc_fd);
		}
	}

	for(int i = 0; i < MAX_CLIENTS; i++) {
		close(pfds[i].fd);
	}
	return 0;
}

void accept_client(struct pollfd* pfds, int* pfds_count) {
	int client_fd = accept(pfds[0].fd, NULL, NULL);
	if(client_fd == -1) {
		perror("Socket accpetion failed");
		return;
	}
	if(*pfds_count == MAX_CLIENTS) return;

	pfds[*pfds_count].fd = client_fd;
	pfds[*pfds_count].events = POLLIN;
	int opts = fcntl(client_fd, F_GETFD, 0);
	fcntl(client_fd, F_SETFD, opts | O_NONBLOCK);
	*pfds_count += 1;
}

// REDO THIS FUNCTION
// Needs to compress the list of client file descriptors
void remove_client(int* client_fds, int fd) {
}

int get_next_client(struct pollfd* pfds, int pfds_count) {
	for(int i = 1; i < pfds_count; i++) {
		if(pfds[i].revents & POLLIN) {
			pfds[i].revents = 0;
			return pfds[i].fd;
		}
	}
	return -1;
}

void propogate_message(void* message_buf, int message_len, struct pollfd* pfds, int pfds_count, int from_fd) {
	printf("Propogating message\n");
	for(int i = 1; i < pfds_count; i++) {
		if(pfds[i].fd != from_fd) {
			write(pfds[i].fd, message_buf, message_len);
		}
	}
}
