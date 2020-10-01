#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define ADDR "irc.chat.twitch.tv"
#define PORT "6667"

SOCKET tc_socket;

int tc_socket_init(void)
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        return -1;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *peer;
    getaddrinfo(ADDR, PORT, &hints, &peer);

    tc_socket = socket(peer->ai_family, peer->ai_socktype, peer->ai_protocol);

    if (tc_socket == INVALID_SOCKET) {
        return -1;
    }

    if (connect(tc_socket, peer->ai_addr, peer->ai_addrlen) == SOCKET_ERROR) {
        return -1;
    }

    freeaddrinfo(peer);

    return 0;
}

void tc_socket_close(void)
{
    closesocket(tc_socket);
    WSACleanup();
}

int tc_send(const char *data)
{
    return send(tc_socket, data, strlen(data), 0);
}

int tc_send_cmd(const char *cmd, const char *arg)
{
    size_t bufsize = strlen(cmd) + strlen(arg) + 3; // 3 is "\r\n\0"
    char buf[bufsize];

    snprintf(buf, sizeof(buf), "%s%s\r\n", cmd, arg);
    return tc_send(buf);
}

int tc_recv(char *buf, int bufsize)
{
    return recv(tc_socket, buf, bufsize, 0);
}

void tc_login(const char *pass, const char *nick)
{
    tc_send_cmd("PASS ", pass);
    tc_send_cmd("NICK ", nick);
}

void tc_join(const char *channel)
{
    tc_send_cmd("JOIN #", channel);
}
