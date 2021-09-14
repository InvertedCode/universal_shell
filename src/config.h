#ifndef _CONFIG_H
#define _CONFIG_H

#include "util.h"

// This is ONLY here for if the protocol is changed. PLEASE DO NOT CHANGE IT UNLESS YOU KNOW WHAT YOURE DOING.
#define SSH_VERSION "2.0"

// This is the version that is sent to the SSH server
#define SSH_IDENT "Universal_Shell"

// This is the comment added to the version string
#define SSH_COMMENT "femboy shell"

// As per RFC-4253
#define SSH_PROTO_STR "SSH-"SSH_VERSION"-"SSH_IDENT" "SSH_COMMENT"\r\n"

#endif
