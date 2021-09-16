#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <stdint.h>

struct ssh_packet {
	int   length;
	char *data;
};

struct name_list {
	int    items;
	char **data;
};

#define SSH_FD 0
int main(int argc, char** argv) {
	// this is currently compiled in, but once it actually works that will change uwu
	char *ssh_host = "127.0.0.1";
	int   ssh_port = 22;

	// some other vars
	int ssh_sock;
	struct sockaddr_in ssh_addr;

	// open the sockets
	ssh_sock = socket(AF_INET, SOCK_STREAM, 0);

	// uwu
	struct pollfd fds[32];
	
	fds[SSH_FD].fd     = ssh_sock;
	fds[SSH_FD].events = POLLIN;

	// put some junk in this structure
	ssh_addr.sin_family      = AF_INET;
	ssh_addr.sin_addr.s_addr = inet_addr(ssh_host);
	ssh_addr.sin_port        = htons(ssh_port);

	// connect to the server
	if(connect(ssh_sock, (struct sockaddr *)&ssh_addr, sizeof(ssh_addr)) != 0) {
		printf("error in connect(): %i\n", errno);
		exit(-1);
	};

	// do the protocol version exchange	
	char remote_version[256];
	read(ssh_sock, remote_version, 255);

	write(ssh_sock, "SSH-2.0-USH_1.0\r\n", 17);

	// main loop
	while (1) {
		poll(fds, 1, 10);

		if (fds[SSH_FD].revents == POLLIN) {
			uint32_t  packet_length;
			uint8_t   padding_length;
			uint8_t  *payload;
			uint8_t  *padding;
			uint8_t  *mac;

			read(ssh_sock, &packet_length, 4);
			packet_length = ntohl(packet_length);

			read(ssh_sock, &padding_length, 1);

			payload = malloc(packet_length - padding_length - 1);
			padding = malloc(padding_length);

			
			read(ssh_sock, payload, packet_length - padding_length - 1);
			read(ssh_sock, padding, padding_length);

			printf("PACKET LENGTH:  %u\n", packet_length);
			printf("PADDING LENGTH: %u\n", padding_length);
			printf("==== BEGIN PAYLOAD ====\n");
			for (int i = 0; i < (packet_length - padding_length - 1); i++) printf("%c", payload[i]);
			printf("\n===== END PAYLOAD =====\n");

			uint8_t type = payload[0];

			printf("TYPE %i\n", type);

			if (type == 20) { // KEX_KEXINIT

			}
		}
	}
}

