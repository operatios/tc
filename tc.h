#ifndef TC_H
#define TC_H

#include <winsock2.h>

int tc_socket_init(SOCKET *s);
void tc_socket_close(SOCKET *s);

int tc_send(SOCKET *s, const char *data);
int tc_send_arg(SOCKET *s, const char *cmd, const char *arg);

int tc_recv(SOCKET *s, char *buf, int bufsize);

void tc_recv_event(SOCKET *s);

void tc_login(SOCKET *s, const char *pass, const char *nick);
void tc_join(SOCKET *s, const char *channel);

#endif /* TC_H */
