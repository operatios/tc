#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define TC_ADDR    "irc.chat.twitch.tv"
#define TC_PORT    "6667"
#define TC_BUFSIZE 2048

enum { OTHER, PING, PRIVMSG };

typedef struct {
    char name[26];
    char *text;
} tc_msg;

tc_msg *tc_msg_new(const char *name, const char *text)
{
    assert(strlen(name) < 26);

    tc_msg *msg = malloc(sizeof(tc_msg));
    strncpy_s(msg->name, sizeof(msg->name), name, sizeof(msg->name) - 1);
    msg->text = _strdup(text);

    return msg;
}

void tc_msg_free(tc_msg *msg)
{
    free(msg->text);
    free(msg);
}

int tc_socket_init(SOCKET *s)
{
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        return -1;
    }

    struct addrinfo hints = { 0 };

    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo *peer;
    getaddrinfo(TC_ADDR, TC_PORT, &hints, &peer);

    *s = socket(peer->ai_family, peer->ai_socktype, peer->ai_protocol);

    if (*s == INVALID_SOCKET) {
        WSACleanup();
        return -1;
    }

    if (connect(*s, peer->ai_addr, peer->ai_addrlen) == SOCKET_ERROR) {
        WSACleanup();
        return -1;
    }

    freeaddrinfo(peer);
    return 0;
}

void tc_socket_close(SOCKET s)
{
    closesocket(s);
    WSACleanup();
}

int tc_send(SOCKET s, const char *data)
{
    char send_buf[strlen(data) + 3]; // 3 is \r\n\0
    sprintf_s(send_buf, sizeof(send_buf), "%s\r\n", data);

    assert(send_buf[sizeof(send_buf) - 3] == '\r');
    assert(send_buf[sizeof(send_buf) - 2] == '\n');
    assert(send_buf[sizeof(send_buf) - 1] == '\0');

    return send(s, send_buf, sizeof(send_buf), 0);
}

int tc_send_args(SOCKET s, int nargs, ...)
{
    va_list args;

    va_start(args, nargs);
    size_t args_len = 0;
    for (int i = 0; i < nargs; i++) {
        args_len += strlen(va_arg(args, char *));
    }
    va_end(args);

    va_start(args, nargs);
    char buf[args_len + 1];
    buf[0] = '\0';

    for (int i = 0; i < nargs; i++) {
        strcat_s(buf, sizeof(buf), va_arg(args, char *));
    }
    va_end(args);

    return tc_send(s, buf);
}

int tc_recv(SOCKET s, char *buf, int bufsize)
{
    return recv(s, buf, bufsize, 0);
}

int tc_get_event_type(const char *line)
{
    if (strcmp(line, "PING :tmi.twitch.tv") == 0) {
        return PING;
    }

    char *cmd_start;
    if (line[0] == '@') {
        char *tags_end = strstr(line, " ");
        assert(tags_end != NULL);

        cmd_start = strstr(tags_end + 1, " ") + 1;

    } else {

        if (line[0] != ':') {
            printf("broken line: '%s'", line);
        }

        assert(line[0] == ':');
        cmd_start = strstr(line, " ") + 1;
    }

    assert(cmd_start != NULL);

    char *cmd_end = strstr(cmd_start + 1, " ") + 1;
    assert(cmd_end != NULL);

    char cmd[cmd_end - cmd_start];

    strncpy_s(cmd, sizeof(cmd), cmd_start, sizeof(cmd) - 1);

    if (strcmp(cmd, "PRIVMSG") == 0) {
        return PRIVMSG;
    }

    return OTHER;
}

tc_msg *tc_parse_msg(const char *line)
{
    assert(line[0] == '@');

    char *tags_start = (char *)line + 1;
    char *tags_end   = strstr(line, " :");

    size_t tags_len = tags_end - tags_start;
    assert(tags_len > 0);

    char tags[tags_len + 1];
    strncpy_s(tags, sizeof(tags), tags_start, sizeof(tags) - 1);

    /*
    char *context = tags;
    char *pair    = strtok_s(context, ";", &context);
    while (pair != NULL) {
        char *key   = strtok_s(pair, "=", &pair);
        char *value = strtok_s(pair, "=", &pair);

        if (strcmp(key, "tmi-sent-ts") == 0) {
            //
        } else if (strcmp(key, "badges") == 0) {
            //
        }

        pair = strtok_s(context, ";", &context);
    }
    */

    char *name_start = tags_end + 2;
    char *name_end   = strstr(name_start, "!");

    size_t name_len = name_end - name_start;
    char name[name_len + 1];

    strncpy_s(name, sizeof(name), name_start, sizeof(name) - 1);

    char *text = strstr(tags_end + 2, " :") + 2;

    return tc_msg_new(name, text);
}

void tc_recv_events(SOCKET s)
{
    char buf[TC_BUFSIZE];
    int size = tc_recv(s, buf, sizeof(buf));

    if (size == -1) {
        return;
    }
    buf[size] = '\0';

    char *context = buf;
    char *line    = strtok_s(context, "\r\n", &context);

    while (line != NULL) {
        int event_type = tc_get_event_type(line);

        switch (event_type) {
        case PING:
            tc_send(s, "PONG :tmi.twitch.tv");
            break;
        case PRIVMSG: {
            tc_msg *msg = tc_parse_msg(line);

            printf("%s: %s\n", msg->name, msg->text);

            tc_msg_free(msg);
            break;
        }
        }

        line = strtok_s(context, "\r\n", &context);
    }
}

void tc_login(SOCKET s, const char *pass, const char *nick)
{
    tc_send_args(s, 2, "PASS ", pass);
    tc_send_args(s, 2, "NICK ", nick);
}

void tc_join(SOCKET s, const char *channel)
{
    tc_send_args(s, 2, "JOIN #", channel);
}

void tc_send_msg(SOCKET s, const char *channel, const char *msg)
{
    tc_send_args(s, 4, "PRIVMSG #", channel, " :", msg);
}
