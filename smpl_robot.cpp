#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>


enum {buf_size = 4096};

struct InfoPlayer {
  int num;
  int state;
  int mon;
  int row;
  int prod;
  int fact;
  int b_fact;
};

struct InfoMarket {
  int lev;
  int mnth;
  int row;
  int min_p;
  int prod;
  int max_p;
  int cur_cl;
};

int SocketErr()
{
  int sockfd;
  do {
    sockfd=socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
      perror("socket");
    }
  } while (sockfd == -1);
  return sockfd;
}

void ReadIpPort(char *ip, char *port)
{
  printf("Enter host's ip and port:");
  scanf("%20s %6s", ip, port);
}

bool FillAddr(struct sockaddr_in &addr, char *ip, char *port)
{
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(port));
  if (!inet_aton(ip, &(addr.sin_addr))) {
    perror("inet_aton");
    printf("You entered wrong ip!\n");
    return false;
  } else return true;
}

void Connection(int sockfd)
{
  struct sockaddr_in addr;
  bool chk;
  char ip[20], port[6];
  do {
    ReadIpPort(ip, port);
    chk = FillAddr(addr, ip, port);
    if (chk) {
      if (0 != connect(sockfd, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("connect");
        printf("Can not connect to the server\n");
        chk = false;
      }
    }
  } while(!chk);
}

void Creation()
{
  int pid;
  char port[6], amnt_pl[20];
  printf("Enter port and amount of players:");
  scanf("%6s %20s", port, amnt_pl);
  do {
    pid = fork();
    if (pid == -1) {
      perror("fork");
    }
  } while(pid == -1);
  if (!pid) {
    execlp("./server.out", "./server.out", port, amnt_pl, NULL);
    perror("execlp");
    exit(1);
  }
}

void WaitStart(int sockfd)
{
  char buf[buf_size];
  int len, i;
  bool chk = false;
  do {
    len = read(sockfd, buf, sizeof(buf)-1);
    for (i=0; (i<len) && (!chk); i++ ) {
      if (buf[i] == '@') chk = true;
    }
  } while(!chk);
}

void FillMarket(struct InfoMarket &market, int *m)
{
  market.lev = m[0];
  market.mnth = m[1];
  market.row = m[2];
  market.min_p = m[3];
  market.prod = m[4];
  market.max_p = m[5];
  market.cur_cl = m[6];
}

void ReadMarket(int sockfd, struct InfoMarket &market)
{
  char buf[buf_size];
  int m[7];
  int len, i, num = 0;
  const char str[]="market\r\n";
  write(sockfd, str, strlen(str));
  do {
    len = read(sockfd, buf, sizeof(buf)-1);
    for(i=0; (i<len) && (num<7); i++) {
      if (buf[i] == '#') {
       sscanf(buf+i+1, "%d", &(m[num]));
       num++;
      }
    }
  } while(num<7);
  FillMarket(market, m);
}
void PrintMarket(struct InfoMarket &market)
{
  printf("Current market level:\n#%d\n", market.lev);
  printf("Current month:\n#%d\n", market.mnth);
  printf("The amount of sold raw:\n#%d\n", market.row);
  printf("The minimum price per row:\n#%d\n", market.min_p);
  printf("The amount of purchased product:\n#%d\n", market.prod);
  printf("The maximum price per product:\n#%d\n", market.max_p);
  printf("The number of active players:\n#%d\n", market.cur_cl);
}

int main(int argc, char **argv)
{
  int sockfd;
  struct InfoMarket market;
  sockfd = SocketErr();
  if (!strcmp(argv[1], "create")) Creation();
  Connection(sockfd);
  WaitStart(sockfd);
  ReadMarket(sockfd, market);
  PrintMarket(market);
  return 0;
}
