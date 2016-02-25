#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>


enum {buf_size = 4096, str_size = 1024};

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
  int max_cl;
  int me;
  struct InfoPlayer *players;
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

void CheckExcep(int len)
{
  if (len == 0) throw "Connection was closed\n";
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
  char ip[str_size], port[str_size];
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

void WaitStart(int sockfd)
{
  char buf[buf_size];
  int len, i;
  bool chk = false;
  do {
    len = read(sockfd, buf, sizeof(buf));
    CheckExcep(len);
    for (i=0; (i<len) && (!chk); i++ ) {
      if (buf[i] == '@') chk = true;
    }
  } while(!chk);
}

void WaitAuction(int sockfd)
{
  char buf[buf_size], str[str_size];
  int len, i;
  bool chk = false;
  sprintf(str, "turn\n");
  write(sockfd, str, strlen(str));
  do {
    len = read(sockfd, buf, sizeof(buf));
    CheckExcep(len);
    for (i=0; (i<len) && (!chk); i++ ) {
      if (buf[i] == '%') chk = true;
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
  int i, num = 0, len, m[7];
  const char str[]="market\n";
  write(sockfd, str, strlen(str));
  do {
    len = read(sockfd, buf, sizeof(buf));
    CheckExcep(len);
    for(i=0; (i<len) && (num<7); i++) {
      if (buf[i] == '#') {
        sscanf(buf+i+1, "%d", &(m[num]));
        num++;
      }
    }
  } while(num < 7);
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

void WhoAmI(int sockfd, struct InfoMarket &market)
{
  const char str[] = "player\n";
  char buf[buf_size];
  int i, len;
  bool chk = false;
  write(sockfd, str, strlen(str));
  do {
    len = read(sockfd, buf, sizeof(buf));
    CheckExcep(len);
    for (i=0; (i<len) && (!chk); i++ ) {
      if (buf[i] == '*') {
        chk = true;
        sscanf(buf+i+1, "%d", &(market.me));
      }
    }
  } while(!chk);
}

void FillPlayer(struct InfoMarket &market, int *m, int indx)
{
  market.players[indx-1].num = indx;
  market.players[indx-1].state = 1;
  market.players[indx-1].mon = m[0];
  market.players[indx-1].row = m[1];
  market.players[indx-1].prod = m[2];
  market.players[indx-1].fact = m[3];
  market.players[indx-1].b_fact = m[4];
}

void ReadPlayers(int sockfd, struct InfoMarket &market)
{
  int i, j, len, num, m[5];
  char str[str_size], buf[buf_size];
  bool leaved;
  for (i=1; i <= market.max_cl; i++) {
    sprintf(str, "player %d\n", i);
    write(sockfd, str, strlen(str));
    leaved = false;
    num = 0;
    do {
      len = read(sockfd, buf, sizeof(buf));
      CheckExcep(len);
      for(j=0; (j<len) && (!leaved) && (num<5); j++) {
        if (buf[j] == '#') {
         sscanf(buf+j+1, "%d", &(m[num]));
         num++;
        }
        if (buf[j] == '^') {
          market.players[i-1].state = 0;
          leaved = true;
        }
      }
    } while((num < 5) && (!leaved));
    if (!leaved) FillPlayer(market, m, i);
  }
}

void PrintPlayers(struct InfoMarket &market)
{
  int i;
  for (i=0; i<market.max_cl; i++) {
    if (market.players[i].state == 1) {
      printf("Information about player *%d:\n", i+1);
      printf("Amount of money:\n#%d\n", market.players[i].mon);
      printf("Amount of row:\n#%d\n", market.players[i].row);
      printf("Amount of products:\n#%d\n", market.players[i].prod);
      printf("Amount of factories:\n#%d\n", market.players[i].fact);
      printf("Amount of factories under constructions:\n#%d\n",
             market.players[i].b_fact);
    }
  }
}

void SetAppl(int sockfd, struct InfoMarket &market)
{
  char str[str_size];
  sprintf(str, "buy 2 %d\n", market.min_p);
  write(sockfd, str, strlen(str));
  sprintf(str, "sell 2 %d\n", market.max_p);
  write(sockfd, str, strlen(str));
  sprintf(str, "prod 2\n");
  write(sockfd, str, strlen(str));
}

int main(int argc, char **argv)
{
  int sockfd;
  struct InfoMarket market;
  sockfd = SocketErr();
  Connection(sockfd);
  try {
    WaitStart(sockfd);
    ReadMarket(sockfd, market);
    market.players =
      (struct InfoPlayer *)malloc(sizeof(struct InfoPlayer)*market.cur_cl);
    market.max_cl = market.cur_cl;
    WhoAmI(sockfd, market);
    for(;;) {
      PrintMarket(market);
      ReadPlayers(sockfd, market);
      PrintPlayers(market);
      SetAppl(sockfd, market);
      WaitAuction(sockfd);
      ReadMarket(sockfd, market);
    }
  }
  catch(const char *str) {
    printf(str);
  }
  return 0;
}
