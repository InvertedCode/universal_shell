#ifndef XU_SSH_H
#define XU_SSH_H
#include <sys/socket.h>

struct _ssh {
    int fd;
};
typedef struct _ssh ssh_t;

struct _ssh_packet {
    int   payload_length;
    void *payload;
};
typedef struct _ssh_packet ssh_packet_t;

ssh_t        *ssh(int sock);
void          ssh_send(ssh_t session, char *payload, int payload_len);
ssh_packet_t  ssh_recv(ssh_t session);

#endif
