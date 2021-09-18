#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <stdint.h>

struct bounded {
	int   length;
	int   newpos;
	void *data;
};
typedef struct bounded bounded_t;

bounded_t *read_namelist(char *at) {
	bounded_t  *l     = malloc(sizeof(bounded_t));
	char      **names = malloc(256);
	uint32_t    len   = 0;

	memcpy(&len, at, 4);
	len = ntohl(len);

	int slen  = 0;
	int start = 4;

	int length = 1;
	
	for (int i = 3; i < len; i++) {
		if (at[i] == ',') {
			slen--;
			char *str = malloc(slen + 1);
			memcpy(str, at + i - slen, slen);
			names[length-1] = str;
			length++;
			slen = 0;
			start = i;
		}
		slen++;
	}

	char *str = malloc(slen + 5);
	memcpy(str, at + start + 1, slen + 3);
	names[length-1] = str;

	l->length = length;
	l->data   = names;
	l->newpos = len+4;
	return l;
}

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
				uint8_t *cookie = malloc(16);
				
				memcpy(cookie, payload + 1, 16);

				int pos = 17;

				bounded_t *kex_algorithms = read_namelist(payload + pos);
				pos += kex_algorithms->newpos;
				bounded_t *server_host_key_algorithms = read_namelist(payload + pos);
				pos += server_host_key_algorithms->newpos;
				bounded_t *encryption_algorithms_client_to_server = read_namelist(payload + pos);
				pos += encryption_algorithms_client_to_server->newpos;
				bounded_t *encryption_algorithms_server_to_client = read_namelist(payload + pos);
				pos += encryption_algorithms_server_to_client->newpos;
				bounded_t *mac_algorithms_client_to_server = read_namelist(payload + pos);
				pos += mac_algorithms_client_to_server->newpos;
				bounded_t *mac_algorithms_server_to_client = read_namelist(payload + pos);
				pos += mac_algorithms_server_to_client->newpos;
				bounded_t *compression_algorithms_client_to_server = read_namelist(payload + pos);
				pos += compression_algorithms_client_to_server->newpos;
				bounded_t *compression_algorithms_server_to_client = read_namelist(payload + pos);
				pos += compression_algorithms_server_to_client->newpos;
				bounded_t *languages_client_to_server = read_namelist(payload + pos);
				pos += languages_client_to_server->newpos;
				bounded_t *languages_server_to_client = read_namelist(payload + pos);
				pos += languages_server_to_client->newpos;

				for (int i = 0; i < kex_algorithms->length; i++) {
					printf("%i: ", i);
					printf("%s\n", ((char **)kex_algorithms->data)[i]);
				}
			}
		}
	}
}
