#include <stdio.h>

#include "tc.h"

int main(int argc, char *argv[])
{
    if (argc != 4) {
        puts("Usage: tc oauth:YOUR_TOKEN user channel");
        return 1;
    }

    int init_result = tc_socket_init();
    if (init_result != 0) {
        puts("Could not initialize a socket");
        return init_result;
    }

    tc_login(argv[1], argv[2]);
    tc_join(argv[3]);

    char buf[4096];
    tc_recv(buf, sizeof(buf));
    printf("%s\n", buf);

    tc_socket_close();
    return 0;
}
