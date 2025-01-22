#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

const int CONNECTION_BACKLOG = 32;
const int DEFAULT_PORT = 25565;
const int READ_BUFFER_SIZE = 512;

int main(void) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1) {
		puts("Unable to create a socket!");
		return -1;
	}

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
	
	if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("Bind failed!");
		close(fd);
		return -1;
	}

	printf("Sucessfully bound server to %s on port %d!\n", ipv4_address, port);

	if(listen(fd, CONNECTION_BACKLOG) != 0) {
		perror("Error trying to start listening for connections!");
		close(fd);
		return -1;
	}
	printf("listening...\n");

	int client_fd = accept(fd, NULL, NULL);
	if(client_fd == -1) {
		perror("Could not accept a connection!\n");
		return -1;
	}

	void* buf = calloc(READ_BUFFER_SIZE, sizeof(char));
	char* message = (char*)calloc(READ_BUFFER_SIZE, sizeof(char));
	for(;;) {
		const int msg_length = read(client_fd, buf, READ_BUFFER_SIZE);
		if(msg_length == -1) {
			perror("Unable to read message from cleint!\n");
		}

		if(msg_length != 0) {
			printf("Received a message %d bytes long!\n", msg_length);
			strncpy(message, (char*)buf, msg_length);
			printf("Message received: %s\n", message);
		}
	}

	close(fd);
	return 0;
}
