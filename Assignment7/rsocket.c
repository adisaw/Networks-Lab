#include "rsocket.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>


int my_sockfd = -1;
int counter = 0;

struct message *receive_buffer = NULL;
int r_start = 0, r_end = 0, r_count = 0;

struct message *send_buffer = NULL;
int s_start = 0, s_end = 0, s_count = 0;

int *recvMsgTable = NULL;

struct unAckMessage *UnAckMsgTable = NULL;
struct unAckMessage *UnAckMsgTableTail=NULL;

int transmissions = 0;

sigset_t set;

void HandleACKMsgRecv(int mID)
{
  struct unAckMessage *prev = NULL;
  struct unAckMessage *temp = UnAckMsgTable;

  while(temp != NULL)
  {
    if(temp->id == mID)
    {
      if(prev == NULL)
      {
        UnAckMsgTable = temp->next;
      }
      else
      {
        prev->next = temp->next;
      }
      if(temp==UnAckMsgTableTail)
      {
      	UnAckMsgTableTail=prev;
      }
      free(temp);
      break;
    }
    prev = temp;
    temp = temp->next;
  }
}

void HandleAppMsgRecv(int id, char *message, struct sockaddr src_addr, socklen_t addrlen, int mlen)
{
  if(recvMsgTable[id] == 1)
  {
    char acknowledgement[3];
    acknowledgement[0]='a';
    acknowledgement[1]=(char)id;
    acknowledgement[2]='\0';
    sendto(my_sockfd, (const char *)acknowledgement, 3, MSG_DONTWAIT, (const struct sockaddr *)&src_addr, addrlen);
  }
  else if (r_count < MAX_PACKETS)
  {
    recvMsgTable[id] = 1;
    bzero(receive_buffer[r_end].msg, BUFFER_SIZE);
    receive_buffer[r_end].len = mlen;
    memcpy(receive_buffer[r_end].msg, message, mlen);
    receive_buffer[r_end].sock_addr = src_addr;
    r_end = (r_end + 1)%MAX_PACKETS;
    r_count++;
    char acknowledgement[3];
    acknowledgement[0]='a';
    acknowledgement[1]=(char)id;
    acknowledgement[2]='\0';
    sendto(my_sockfd, (const char *)acknowledgement, 3, MSG_DONTWAIT, (const struct sockaddr *)&src_addr, addrlen);
  }
}

void HandleReceive()
{
  struct sockaddr src_addr;
  char buffer[BUFFER_SIZE];
  socklen_t addrlen;
  int len, id;

  while ((len = recvfrom(my_sockfd, (void  *)buffer, BUFFER_SIZE, MSG_DONTWAIT, &src_addr, &addrlen)) >= 0)
  {
    if (dropMessage(P))
      continue;
    id = (int)(buffer[1]);
    if(buffer[0] == 'a')
    {
      HandleACKMsgRecv(id);
    }
    else
    {
      HandleAppMsgRecv(id, buffer, src_addr, addrlen, len);
    }
  }

  if (errno != EAGAIN && errno != EWOULDBLOCK)
  {
    perror("\nError in receiving packets ");
    exit(EXIT_FAILURE);
  }
}

void HandleRetransmit()
{
  struct unAckMessage* temp;
  temp = UnAckMsgTable;
  int k = 0;

  struct  timeval current;
  gettimeofday(&current, NULL);
  struct timeval result;

  while(temp != NULL)
  {
    result.tv_sec = current.tv_sec-(temp->sendTime).tv_sec;
    if(result.tv_sec >= T)
    {
      k = sendto(my_sockfd, (const char*)temp->pack.msg, temp->pack.len, MSG_DONTWAIT, (const struct sockaddr*)(&(temp->pack.sock_addr)),
          sizeof(temp->pack.sock_addr));
      if (k < 0)
      {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
          perror("\nError in receiving packets ");
          exit(EXIT_FAILURE);
        }
        break;
      }
      transmissions++;
      (temp->sendTime).tv_usec = current.tv_usec;
      (temp->sendTime).tv_sec = current.tv_sec;
    }
    temp = temp->next;
  }
}

void HandleTransmit()
{
  int i = 0, idx = 0;
  struct  timeval current;
  gettimeofday(&current, NULL);
  struct unAckMessage *temp = NULL;
  for (; i < s_count; ++i)
  {
    idx = (i+s_start)%MAX_PACKETS;
    int k = sendto(my_sockfd, (const char*)send_buffer[idx].msg, send_buffer[idx].len, MSG_DONTWAIT,
        (const struct sockaddr*)(&send_buffer[idx].sock_addr), sizeof(send_buffer[idx].sock_addr));
    if (k < 0)
    {
      if (errno != EAGAIN && errno != EWOULDBLOCK)
      {
        perror("\nError in receiving packets ");
        exit(EXIT_FAILURE);
      }
      break;
    }
    else
    {
      transmissions++;
      temp = (struct unAckMessage *)malloc(sizeof(struct unAckMessage));
      temp->id = (int)send_buffer[idx].msg[1];
      (temp->sendTime).tv_usec = current.tv_usec;
      (temp->sendTime).tv_sec = current.tv_sec;
      temp->pack = send_buffer[idx];
      temp->next=NULL;
      if(UnAckMsgTable==NULL)
      {
      	UnAckMsgTableTail=temp;
      	UnAckMsgTable=temp;
      }
      else
      {
      	UnAckMsgTableTail->next=temp;
      	UnAckMsgTableTail=temp;
      }

    }
  }
  s_count = s_count - i;
  s_start = (s_start+i)%MAX_PACKETS;
}

void signal_handler(int s)
{
  HandleReceive();
  HandleRetransmit();
  HandleTransmit();
}

int r_socket(int domain, int type, int protocol)
{
  srand(time(0));

  if (type != SOCK_MRP || my_sockfd >= 0)
    return -1;
  my_sockfd = socket(domain, SOCK_DGRAM, protocol);
  if(my_sockfd < 0)
  {
    return -1;
  }

  receive_buffer = (struct message *)malloc(sizeof(struct message) * MAX_PACKETS);
  send_buffer = (struct message *)malloc(sizeof(struct message) * MAX_PACKETS);;
  UnAckMsgTable = NULL;
  recvMsgTable = (int *)malloc(sizeof(int) * MAX_PACKETS);
  bzero(recvMsgTable, sizeof(int) * MAX_PACKETS);

  signal(SIGALRM, signal_handler);
  sigemptyset(&set);
  sigaddset(&set, SIGALRM);

  return my_sockfd;
}

int r_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  if (my_sockfd != sockfd)
    return -1;

  int e = bind(sockfd, addr, addrlen);
  if (e != 0)
    return e;

  struct itimerval timer;
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = FREQ;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = FREQ;
  setitimer (ITIMER_REAL, &timer, NULL);

  return 0;
}

ssize_t r_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
{
  if (my_sockfd != sockfd)
    return -1;

  while (1)
  {
    if (r_count > 0)
      break;
  }
  sigprocmask(SIG_BLOCK, &set, NULL);

  size_t temp = receive_buffer[r_start].len - 2;
  if (len < temp)
    temp = len;

  memcpy(buf, receive_buffer[r_start].msg + 2, temp);

  *src_addr = receive_buffer[r_start].sock_addr;
  *addrlen = sizeof(receive_buffer[r_start].sock_addr);

  r_count--;
  r_start = (r_start+1)%MAX_PACKETS;

  sigprocmask(SIG_UNBLOCK, &set, NULL);

  return temp;
}

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
  if (my_sockfd != sockfd)
    return -1;

  while (1)
  {
    if (s_count < MAX_PACKETS)
      break;
  }
  sigprocmask(SIG_BLOCK, &set, NULL);

  send_buffer[s_end].msg[0] = 'd';
  send_buffer[s_end].msg[1] = (char)counter;
  counter++;

  if (len > BUFFER_SIZE-2)
    len = BUFFER_SIZE-2;
  memcpy(send_buffer[s_end].msg+2, buf, len);
  send_buffer[s_end].len = len + 2;
  send_buffer[s_end].sock_addr = *dest_addr;

  s_count++;
  s_end = (s_end+1)%MAX_PACKETS;

  sigprocmask(SIG_UNBLOCK, &set, NULL);

  return 0;
}

int r_close(int sockfd)
{
  struct itimerval timer;
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;
  setitimer (ITIMER_REAL, &timer, NULL);

  if (my_sockfd != sockfd)
    return -1;
  int e = close(sockfd);
  if (e != 0)
    return -1;

  my_sockfd = -1;
  counter = 0;

  free(receive_buffer);
  receive_buffer = NULL;
  r_start = 0;
  r_end = 0;
  r_count = 0;

  free(send_buffer);
  send_buffer = NULL;
  s_start = 0;
  s_end = 0;
  s_count = 0;

  free(recvMsgTable);
  recvMsgTable = NULL;

  struct unAckMessage *temp1 = UnAckMsgTable, *temp2 = NULL;
  while(temp1 != NULL)
  {
    temp2 = temp1;
    temp1 = temp1->next;
    free(temp2);
  }
  UnAckMsgTable = NULL;

  printf("Number of Transmissions = %d\n", transmissions);
  transmissions = 0;

  return 0;
}

int dropMessage(float p)
{
  float x = (float)rand()/(float)(RAND_MAX);
  return x < p;
}
