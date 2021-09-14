#include "config.h"
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include "ssh.h"

struct _args {
    int ftp;
    int ftp_port;
    int ssh;
    int ssh_port;
    struct sockaddr_in ftp_addr;
    struct sockaddr_in ssh_addr;
};
typedef struct _args args_t;

int main(int argc, char** argv) {
    args_t args;
    memset(&args, 0, sizeof(args));
    char*  addr;
    
    // defaults
    args.ftp      = 0;
    args.ftp_port = 53;

    // parsing
    addr = argv[1];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ftp") == 0) {
            args.ftp = 1; i++;
            sscanf(argv[i],"%i", &args.ftp_port);
        } else if (strcmp(argv[i], "--ssh") == 0) {
            args.ssh = 1; i++;
            sscanf(argv[i],"%i", &args.ssh_port);
            args.ssh_addr.sin_port = htons(args.ssh_port);
            printf("SSH_PORT %i\r\n", args.ssh_port);
        } else if (strcmp(argv[i], "--ipv4") == 0) {
            i++;
            args.ssh_addr.sin_family = AF_INET;
            inet_pton(AF_INET, argv[i], &args.ssh_addr.sin_addr);
            inet_pton(AF_INET, argv[i], &args.ftp_addr.sin_addr);
        }
    }

    // setup shell
    struct termios backup;
    struct termios term;

    tcgetattr(0, &backup);
    tcgetattr(0, &term);

    cfmakeraw(&term);
    setvbuf(stdout, NULL, _IONBF, 0);

    tcsetattr(0, 0, &term);

    // setup some crap
    char   ib[1024]; // inbound data
    char   ob[1024]; // outbound data
    char   sc[1024]; // scratch buffer for read
    
    int    ib_len; // length of ib
    int    ob_len; // length of ob
    int    sc_len; // length of sc
    
    /*
        this actually deserves a bit of explaining, so basically
        the shell has multiple "modes" it can be in.
        0 - internal command (like the thing you get when you press esc in vi)
        1 - normal (you get it)
    */
    int    mode;

    mode = 0;

    int ftp_connected, ftp_giveup, ftp_hassock, ftp_sock;
    int ssh_connected, ssh_giveup, ssh_hassock, ssh_sock;
    int stdin_fd, ssh_fd;

    // fds
    struct pollfd connections[32];
    memset(&connections, 0, sizeof(connections));

    int    fdtype[32];

    connections[0].fd     = STDIN_FILENO;
    connections[0].events = POLLIN;
    connections[1].fd     = socket(AF_INET, SOCK_STREAM, 0);
    connections[1].events = POLLIN;

    ssh_t *ssh_session;

    printf("PHASE: CONNECT\r\n");
    if (args.ssh) {
        ssh_sock = connections[1].fd;
        
        if (connect(ssh_sock, (struct sockaddr *)&args.ssh_addr, sizeof(args.ssh_addr)) < 0) {
            printf("Unable to connect to remote host [ERROR %i IN connect()]\r\n", errno);
            goto ssh_connect_die;
        }

        ssh_session = ssh(ssh_sock);
    }

    ssh_connect_die:

    // main loop
    while (1) {
        poll(connections, 2, 100);

        if (connections[0].revents & POLLIN) {
            memset(&sc, 0, sizeof(sc));
            sc_len = read(STDIN_FILENO, &sc, sizeof(sc) - 1);
            for (int i = 0; i < sc_len; i++) {
                if (sc[i] == 3) {
                    goto leave;
                }
            }
        }

        if (connections[1].revents & POLLIN) {
            memset(&sc, 0, sizeof(sc));
            sc_len = read(ssh_sock, &sc, sizeof(sc));
            for (int ii = 0; ii < sc_len; ii++) {
                printf("%c ", sc[ii]);
            }
        }
    }
    
    leave: // a thing to exit, we could break, but this allows us to leave from anywhere, even nested loops

    // restore shell
    tcsetattr(0, 0, &backup);

    return 0;
}
