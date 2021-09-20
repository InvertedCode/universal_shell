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

#define LIST_END "\xFF\xFF\xFF\xFF"

const char *kex_ke[2] = { // key exchange
	"none", LIST_END
};
const char *kex_hk[2] = { // server host key
	"none", LIST_END
};
const char *kex_se[2] = { // serverbound encryption
	"none", LIST_END
};
const char *kex_ce[2] = { // clientbound encryption
	"none", LIST_END
};
const char *kex_sm[2] = { // serverbound mac
	"none", LIST_END
};
const char *kex_cm[2] = { // clientbound mac
	"none", LIST_END
};
const char *kex_sc[2] = { // serverbound compression
	"none", LIST_END
};
const char *kex_cc[2] = { // clientbound compression
	"none", LIST_END
};
const char *kex_sl[2] = { // serverbound languages
	"none", LIST_END
};
const char *kex_cl[2] = { // clientbound languages
	"none", LIST_END
};
const void *kex_al[10] = {&kex_ke, &kex_hk, &kex_se, &kex_ce, &kex_sm, &kex_cm, &kex_sc, &kex_sc, &kex_sl, &kex_cl};

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

void *encode_namelist(char **data, int length) {
	void *list    = malloc(3 + length); // 4 bytes for the length and some bytes for the commas
	int   datalen = 3 + length;

	for (int i = 0; i < length; i++) {

	}

	bounded_t *encoded = malloc(sizeof(bounded_t));
	encoded->length = datalen;
	encoded->data   = list;
	return list;
}

void send_ssh_packet(int fd, uint8_t *data, int data_length, int cipher_block_size) {
	void *packet = malloc(4);

	uint8_t  padding_length = 8 + ((8 < cipher_block_size) ? cipher_block_size : 8) - ((5 + data_length) % ((8 < cipher_block_size) ? cipher_block_size : 8));
	uint32_t packet_length = htonl(1 + padding_length + data_length);
	printf("PADDING_LENGTH %i\n", padding_length);
	printf("PACKET_LENGTH  %i\n", packet_length);

	packet = realloc(packet, packet_length+4);

	memcpy(packet, &packet_length, 4);
	memcpy(packet+4, &padding_length, 1);
	memcpy(packet+5, data, data_length);

	write(fd, packet, packet_length+4);
}

#define SSH_FD 0
int main(int argc, char** argv) {
	// this is currently compiled in, but once it actually works that will change uwu
	char *ssh_host = "127.0.0.1";
	int   ssh_port = 2222;

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

			if (type == 20) { // MSG_KEXINIT
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
				};

				int   res_length = 17;
				void *res       = malloc(res_length);
     
				for (int i = 0; i < 10; i++) {
					int ii = 0;
					int lo = res_length; // the offset of the length
					int len = 0;

					res_length += 4;
					res = realloc(res, res_length);
					memset(res + res_length - 4, 0, 4);

					while (strcmp(((char***)kex_al)[i][ii], LIST_END) != 0) {
						res_length += strlen(((char***)kex_al)[i][ii]) + 1;
						res         = realloc(res, res_length);

						len += strlen(((char***)kex_al)[i][ii]) + 1;

						memset(res + res_length - 1, ',', 1);
						memcpy(res + res_length - 1 - strlen(((char***)kex_al)[i][ii]), (((char***)kex_al)[i][ii]), strlen(((char***)kex_al)[i][ii]));
						//printf("(len %i) (newthing %s)\n", res_length, ((char***)kex_al)[i][ii]);
						ii++;
					}

					if (ii) {
						res_length--;
						len--;

						len = htonl(len);

						memcpy(res + lo, &len, 4);
						memset(res + res_length, 0, 1);

						res = realloc(res, res_length);
					}
				}

				((char*)res)[0] = 20;

				res = realloc(res, res_length + 5);
				((char*)res)[res_length] = 0;
				memset(res+res_length, 0, 4); // reserved
				res_length += 5;

				printf("\n");
				for (int i = 0; i < res_length; i++) printf("%i ", ((char*)res)[i]);
				printf("\n");

				send_ssh_packet(ssh_sock, res, res_length, 0);
			}
		}
	}
}
