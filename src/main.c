#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	// this is currently compiled in, but once it actually works that will change uwu
	char *ssh_host = "127.0.0.1";
	int   ssh_port = 22;

	// some other vars
	int ssh_sock;
	struct sockaddr_in ssh_addr;

	// open the socket
	if((ssh_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("error in scoket(): %i\n", errno);
		exit(-1);
	}

	// put some junk in this structure
	ssh_addr.sin_family=AF_INET;
	ssh_addr.sin_addr.s_addr = inet_addr(ssh_host);
	ssh_addr.sin_port = htons(ssh_port);

	// connect to the server
	if(connect(ssh_sock, (struct sockaddr *)&ssh_addr, sizeof(ssh_addr)) < 0) {
		printf("error in connect(): %i\n", errno);
		exit(-1);
	};
}
