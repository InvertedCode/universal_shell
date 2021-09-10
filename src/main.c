#include "config.h"
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

struct _args {
    int dns;
    int dns_port;
};

typedef struct _args args_t;

extern void print(const char *, ...);

int main(int argc, char** argv) {
    args_t args;
    char*  addr;
    int    dns_sock;

    // defaults
    args.dns      = 0;
    args.dns_port = 53;

    // parsing
    addr = argv[1];

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--dns") == 0) {
            args.dns = 1; i++;
            sscanf(argv[i],"%i", &args.dns_port);
        }
    }

    // setup shell
    struct termios backup;
    struct termios term;

    tcgetattr(0, &backup);
    tcgetattr(0, &term);

    cfmakeraw(&term);

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
        0 - shell command (like the thing you get when you press esc in vi)
        1 - dns (you get it)
    */
    int    mode;

    /*
        basically just a backup of mode for exiting mode 0
    */
    int    retmode;

    mode = 0;
    retmode = 0;

    // main loop
    while (1) {
        // this is HORRIBLE. WHY DO YOUNEED TO INPUT TO GET OUHOASHJCNKASLDKJCNALSKDNFUCKYOUX
        sc_len = read(0, &sc, sizeof(sc));
        for (int i = 0; i < sc_len; i++) {
            if ((sc[i] == 194) && (sc[i] == 155)) {
                // SHIFT-ESC: the key to do the thing idk its like escape in vi
                if (mode == 0) {
                    mode = retmode;
                } else {
                    retmode = mode;
                    mode = 0;
                    // now we're ready to do stuff like configure the shell uwu
                }
            } else if (sc[i] == 3) {
                // loks like we need to exit early
                goto leave;
            } else {
                print("%i ", sc[i]);
            }
        }
    }
    
    leave: // a thing to exit, we could break, but this allows us to leave from anywhere, even nested loops

    // restore shell
    tcsetattr(0, 0, &backup);

    return 0;
}
