#include <stdio.h>
#include <signal.h>

#include "tc.h"
#include "colors.h"

static SOCKET s;

static void handle_sigint(int _)
{
    (void)_;
    tc_socket_close(s);
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        puts("Usage: tc channel [oauth:token, username]");
        return 1;
    }

    signal(SIGINT, handle_sigint);
    enable_colors();

    int err = tc_socket_init(&s);

    if (err != 0) {
        puts("Could not initialize a socket");
        exit(err);
    }

    if (argc < 4) {
        puts("Logging in anonymously");
        // PASS can be any non-empty string, NICK has to be justinfan*
        tc_login(s, "anon", "justinfan65");
    } else {
        tc_login(s, argv[2], argv[3]);
    }

    tc_join(s, argv[1]);
    tc_send(s, "CAP REQ :twitch.tv/tags");
    tc_send(s, "CAP REQ :twitch.tv/commands");

    while (1) {
        tc_recv_events(s);
    }
}
