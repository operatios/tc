#include <stdio.h>

#include <winsock2.h>

#include "tc.h"

int main(int argc, char *argv[])
{
    if (argc == 1) {
        puts("Usage: tc channel [oauth:token, username]");
        return 1;
    }

    SOCKET sock;
    int init_result = tc_socket_init(&sock);

    if (init_result != 0) {
        puts("Could not initialize a socket");
        return init_result;
    }

    if (argc < 4) {
        puts("Logging in anonymously");
        // PASS can be any non-empty string, NICK has to be justinfan*
        tc_login(&sock, "anon", "justinfan65");
    } else {
        tc_login(&sock, argv[2], argv[3]);
    }

    tc_join(&sock, argv[1]);

    tc_send(&sock, "CAP REQ :twitch.tv/tags");
    tc_send(&sock, "CAP REQ :twitch.tv/commands");

    // Add signal handle to call tc_socket_close() before exiting
    while (1) {
        tc_recv_event(&sock);
    }

    tc_socket_close(&sock);
    return 0;
}
