#include <string.h>
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

const int MESSAGE_BUF_LEN = 512;

int main(void) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1) {
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
	
	if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("Connection to host failed!");
		close(fd);
		return -1;
	}

	void* message_buf = calloc(MESSAGE_BUF_LEN, sizeof(char));
	for(;;) {
		int message_len = read(0, message_buf, MESSAGE_BUF_LEN);
		if(message_len == -1) {
			perror("Failed to send message!\n");
			continue;
		}

		if(write(fd, message_buf, message_len) == -1) {
			perror("Message writing failed!\n");
			continue;
		}
	}
	
	close(fd);
	return 0;
}
