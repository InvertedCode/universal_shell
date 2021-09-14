#include "ssh.h"
#include "config.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

ssh_t *ssh(int sock) {
    ssh_t session;
    session.fd = sock;

    write(sock, SSH_PROTO_STR, sizeof(SSH_PROTO_STR));

    char remote_ident[256];
    char test[5];
    
    memset(&remote_ident, 0, 256);
    memset(&test, 0, 5);
    
    read(sock, &remote_ident, 255);
    
    memcpy(&test, &remote_ident, 4);

    if (strcmp(test, "SSH-") != 0) {
        printf("Error | Remote returned invalid identification string\r\n");
        printf("      L Expected it to start with SSH- but instead it started with %s\r\n", test);
        close (sock);
        return -1;
    }

    // we dont need those newlines
    printf("REMOTE VERSION - %sLOCAL VERSION - %s", remote_ident, SSH_PROTO_STR);
    
    ssh_send(session, "doing your mom", 15);
    ssh_recv(session);

    printf("SSH READY\r\n");
    
    return &session;
}

void ssh_send(ssh_t session, char *packet_payload, int payload_length) {
    int padding_len;
    int payload_len;

    padding_len = 4;
    payload_len = payload_length;

    char buf[35000];
    
    memset(&buf, 0, 35000);

    char     *packet         = &buf;
    uint32_t *packet_length  = &buf;
    uint8_t  *padding_length = &buf + 4;
    char     *payload        = &buf + 5;
    char     *padding        = &buf + 5 + payload_length;
    char     *mac            = &buf + 5 + payload_length + padding_len;

    packet_length[0] = payload_len + padding_len + 1; // trol

    for (int i = 0; i < 35000; i++) printf("%c", packet[i]);

    write(session.fd, &buf, packet_length);
}

ssh_packet_t ssh_recv(ssh_t session) {
    ssh_packet_t parsed;
    int   packet_len;
    int   padding_len;
    int   payload_len;
    char *payload;
    char *junk;

    uint8_t raw[4];

    // reading numbers is pain
    memset(&raw, 0, 4);
    recv(session.fd, &packet_len  + 3, 1, MSG_WAITALL);
    recv(session.fd, &packet_len  + 2, 1, MSG_WAITALL);
    recv(session.fd, &packet_len  + 1, 1, MSG_WAITALL);
    recv(session.fd, &packet_len  + 0, 1, MSG_WAITALL);
    recv(session.fd, &raw         + 0, 1, MSG_WAITALL);
    memcpy(&padding_len, &raw, 4);

    /* 
        ????
        for some reason this only worked when i multiplied it by 4
        what the fuck
        2 HOURS wasted on this 
    */
    payload_len = packet_len * 4 - padding_len;
    payload = malloc(payload_len);
    
    junk = malloc(padding_len);

    recv(session.fd, payload, payload_len, 0);
    recv(session.fd, junk, payload_len, 0);

    // printf("INBOUND SSH\r\n");
    // printf("PACKET  LEN %u\r\n", packet_len);
    // printf("PADDING LEN %u\r\n", padding_len);
    // printf("PAYLOAD LEN %u\r\n", payload_len);
    // printf("START PAYLOAD\r\n");
    // for (int i = 0; i < payload_len; i++) printf("%c", payload[i]);
    // printf("\r\nEND PAYLOAD\r\n");
    // printf("END INBOUND SSH\r\n");

    parsed.payload = payload;
    parsed.payload_length = payload_len;
    return parsed;
}
