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

// @todo: replace strstr with strchr

/* Event types, we only care about PING and PRIVMSG */
enum { OTHER, PING, PRIVMSG };

/* Badges flags */
enum {
    SUB         = 1 << 0,
    MOD         = 1 << 1,
    BROADCASTER = 1 << 2,
    GLOBAL_MOD  = 1 << 3,
    STAFF       = 1 << 4,
    ADMIN       = 1 << 5,
};

typedef struct {
    char name[26];
    char *text;
} tc_msg;

tc_msg *tc_msg_new(const char *name, const char *text)
{
    assert(strlen(name) < 26);
    tc_msg *msg = malloc(sizeof(tc_msg));

    strncpy_s(msg->name, sizeof(msg->name), name, sizeof(msg->name) - 1);
    assert(sizeof(msg->name) == 26);

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
        cmd_start      = strstr(tags_end + 1, " ") + 1;

    } else {
        assert(line[0] == ':');
        cmd_start = strstr(line, " ") + 1;
    }

    char *cmd_end = strstr(cmd_start, " ");

    char cmd[(cmd_end - cmd_start) + 1];
    strncpy_s(cmd, sizeof(cmd), cmd_start, sizeof(cmd) - 1);

    if (strcmp(cmd, "PRIVMSG") == 0) {
        return PRIVMSG;
    }
    return OTHER;
}

tc_msg *tc_parse_msg(char *line)
{
    assert(line[0] == '@');

    char *tags_start = line + 1;
    char *tags_end   = strstr(line, " :");

    ptrdiff_t tags_len = tags_end - tags_start;

    /* tag parsing

    char tags[tags_len + 1];
    strncpy_s(tags, sizeof(tags), tags_start, sizeof(tags) - 1);

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

    ptrdiff_t name_len = name_end - name_start;
    char name[name_len + 1];

    strncpy_s(name, sizeof(name), name_start, sizeof(name) - 1);

    char *text = strstr(tags_end + 2, " :") + 2;

    return tc_msg_new(name, text);
}

void tc_recv_events(SOCKET s)
{
    size_t bufsize = TC_BUFSIZE;
    char *buf      = malloc(sizeof(char) * bufsize);

    int offset = tc_recv(s, buf, bufsize);

    if (offset == -1) {
        return;
    }

    // clang-format off
    while (buf[offset - 1] != '\0' &&
           buf[offset - 2] != '\n' &&
           buf[offset - 3] != '\r')
    // clang-format on
    {

        if (offset >= (int)bufsize) {
            bufsize *= 2;
            buf = realloc(buf, sizeof(char) * bufsize);
        }

        int result = tc_recv(s, buf + offset, bufsize - offset);

        if (result == -1) {
            break;
        }

        offset += result;
    }

    char *line_offset = buf;

    while (line_offset - buf < offset) {
        char *line_end     = strstr(line_offset, "\r\n\0");
        ptrdiff_t line_len = line_end - line_offset;

        char token[line_len + 1];
        strncpy_s(token, sizeof(token), line_offset, sizeof(token) - 1);

        int event_type = tc_get_event_type(token);

        switch (event_type) {
        case PING:
            tc_send(s, "PONG :tmi.twitch.tv");
            break;
        case PRIVMSG: {
            tc_msg *msg = tc_parse_msg(token);
            printf("%s:%s\n", msg->name, msg->text);
            tc_msg_free(msg);
            break;
        }
        }

        line_offset = line_end + 3;
    }
    free(buf);
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
