#ifndef TC_H
#define TC_H

int tc_socket_init(void);
void tc_socket_close(void);

int tc_send(const char* data);
int tc_send_cmd(const char* cmd, const char* arg);

int tc_recv(char* buf, int bufsize);

void tc_login(const char* pass, const char* nick);
void tc_join(const char* channel);

#endif // TC_H
