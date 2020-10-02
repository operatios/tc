#ifndef TC_H
#define TC_H

#include <winsock2.h>

int tc_socket_init(SOCKET *s);
void tc_socket_close(SOCKET s);

int tc_send(SOCKET s, const char *data);

void tc_login(SOCKET s, const char *pass, const char *nick);
void tc_join(SOCKET s, const char *channel);
void tc_send_msg(SOCKET s, const char *channel, const char *msg);

void tc_recv_events(SOCKET s);

#endif /* TC_H */
