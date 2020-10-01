#include <assert.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define TC_RECV_BUF 2048

#define TC_ADDR "irc.chat.twitch.tv"
#define TC_PORT "6667"

int tc_socket_init(SOCKET *s)
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
    getaddrinfo(TC_ADDR, TC_PORT, &hints, &peer);

    *s = socket(peer->ai_family, peer->ai_socktype, peer->ai_protocol);

    if (*s == INVALID_SOCKET) {
        return -1;
    }

    if (connect(*s, peer->ai_addr, peer->ai_addrlen) == SOCKET_ERROR) {
        return -1;
    }

    freeaddrinfo(peer);
    return 0;
}

void tc_socket_close(SOCKET *s)
{
    closesocket(*s);
    WSACleanup();
}

int tc_send(SOCKET *s, const char *data)
{
    char send_buf[strlen(data) + 3]; // 3 is \r\n\0
    snprintf(send_buf, sizeof(send_buf), "%s\r\n", data);

    printf("-> `%s`\n", data);
    assert(send_buf[sizeof(send_buf) - 3] == '\r');
    assert(send_buf[sizeof(send_buf) - 2] == '\n');
    assert(send_buf[sizeof(send_buf) - 1] == '\0');

    return send(*s, send_buf, strlen(send_buf), 0);
}

int tc_send_arg(SOCKET *s, const char *cmd, const char *arg)
{
    char buf[strlen(cmd) + strlen(arg) + 1];
    snprintf(buf, sizeof(buf), "%s%s", cmd, arg);

    return tc_send(s, buf);
}

int tc_recv(SOCKET *s, char *buf, int bufsize)
{
    return recv(*s, buf, bufsize, 0);
}

void tc_recv_event(SOCKET *s)
{
    char buf[TC_RECV_BUF];
    int size = tc_recv(s, buf, sizeof(buf));
    buf[size] = '\0';

    if (strcmp(buf, "PING :tmi.twitch.tv\r\n") == 0) {
        tc_send(s, "PONG :tmi.twitch.tv");
    } else {
        printf("%s", buf);
    }
}

void tc_login(SOCKET *s, const char *pass, const char *nick)
{
    tc_send_arg(s, "PASS ", pass);
    tc_send_arg(s, "NICK ", nick);
}

void tc_join(SOCKET *s, const char *channel)
{
    tc_send_arg(s, "JOIN #", channel);
}
