#ifndef _RSOCKET_H
#define _RSOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define T 2
#define P 0.5
#define FREQ 1000
#define BUFFER_SIZE 102
#define MAX_PACKETS 100
#define SOCK_MRP 10057

struct message
{
	char msg[BUFFER_SIZE];
	size_t len;
	struct sockaddr sock_addr;
};

struct unAckMessage
{
	struct message pack;
	struct timeval sendTime;
	int id;
	struct unAckMessage *next;
};

int r_socket(int, int, int);
int r_bind(int, const struct sockaddr *, socklen_t);
ssize_t r_recvfrom(int, void *, size_t, int, struct sockaddr *, socklen_t *);
ssize_t r_sendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);
int r_close(int);
int dropMessage(float);

#endif
