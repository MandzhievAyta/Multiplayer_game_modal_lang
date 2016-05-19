#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include "robot.h"
#include "interpreter.h"


enum { buf_size = 4096, str_size = 1024 };
enum { find_info_auct = 0, info_auct_beg = 1, info_auct_end = 2 };
enum { sold_mode = '#', bought_mode = '~' };
enum { start_sym = '@', auct_bord_sym = '%', info_sym = '#', win_sym = '|' };

class GameOverResultEx {
  enum { len = 2048 };
  char str[len];
  int winner;
public:
  GameOverResultEx(char *buf, int size, char bor_sym);
  void PrintGameOverResultEx() const { printf(str); }
};

GameOverResultEx::GameOverResultEx(char *buf, int size, char detector)
{
  int i;
  winner = 0;
  for (i = 0; i<size; i++) {
    if (buf[i] == detector) {
      sscanf(buf+i+1, "%d", &winner);
      break;
    }
  }
  if (winner != 0) {
    sprintf(str, "You won!\n");
  } else {
    sprintf(str, "You lose!\n");
  }
}
/*
class SmartBufer {
private:
  int sockfd;
  int cur_size;
  char buf[buf_size];
public:
  SmartBufer(int fd): sockfd(fd), cur_size(0) {}
  char *GetString();
};
*/
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

bool FillAddr(struct sockaddr_in &addr, char *ip, char *port)
{
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(port));
  if (!inet_aton(ip, &(addr.sin_addr))) {
    perror("inet_aton");
    printf("You entered wrong ip!\n");
    return false;
  } else
    return true;
}

void Connection(int sockfd, char *ip, char *port)
{
  struct sockaddr_in addr;
  bool chk;
  do {
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
    if (len == 0) {
      throw GameOverResultEx(buf, buf_size, win_sym);
    }
    for (i=0; (i<len) && (!chk); i++ ) {
      if (buf[i] == start_sym) {
        chk = true;
      }
    }
  } while(!chk);
}

inline void NullBet(InfoBet *bet, int amnt)
{
  int i;
  for (i = 0; i < amnt; i++)
    bet[i].state = 0;
}

void FillBet(int *m, InfoBet &ar)
{
  ar.state = 1;
  ar.amnt = m[0];
  ar.num = m[1];
  ar.price = m[2];
}

void ReadAuction(InfoMarket &market,const char *buf, int len, char mode)
{
  int i, m[3], j = 0,  num = 0;
  InfoBet *bet;
  if (mode == '#') {
    bet = market.sold;
  } else {
    bet = market.bought;
  }
  NullBet(bet, market.max_cl);
  for (i = 0; i < len; i++){
    if (buf[i] == mode) {
      sscanf(buf+i+1, "%d", &m[j]);
      j++;
      if (j == 3) {
        FillBet(m, bet[num]);
        num++;
        j = 0;
      }
    }
  }
}

void WaitReadAuction(int sockfd, InfoMarket &market)
{
  char buf[buf_size], str[str_size];
  int cur_size = 0, lenbuf = 0, i, beg, leninfo, chk = find_info_auct;
  sprintf(str, "turn\n");
  write(sockfd, str, strlen(str));
  do {
    lenbuf = read(sockfd, buf + cur_size, sizeof(buf) - cur_size);
    cur_size += lenbuf;
    if (lenbuf == 0) {
      throw GameOverResultEx(buf, buf_size, win_sym);
    }
    for (i = cur_size - lenbuf; i < cur_size; i++) {
      if (buf[i] == auct_bord_sym) {
        if (chk == find_info_auct) {
          chk = info_auct_beg;
          beg = i+1;
        } else if (chk == info_auct_beg) {
          chk = info_auct_end;
          leninfo = i - beg;
          break;
        }
      }
    }
  } while(chk != info_auct_end);
  ReadAuction(market, buf + beg, leninfo, sold_mode);
  ReadAuction(market, buf + beg, leninfo, bought_mode);
}

void FillMarket(InfoMarket &market, int *m)
{
  market.lev = m[0];
  market.mnth = m[1];
  market.row = m[2];
  market.min_price = m[3];
  market.prod = m[4];
  market.max_price = m[5];
  market.cur_cl = m[6];
}

void ReadMarket(int sockfd, InfoMarket &market)
{
  char buf[buf_size];
  int i, num = 0, len, m[7];
  const char str[]="market\n";
  write(sockfd, str, strlen(str));
  do {
    len = read(sockfd, buf, sizeof(buf));
    if (len == 0) {
      throw GameOverResultEx(buf, buf_size, win_sym);
    }
    for(i = 0; (i < len) && (num < 7); i++) {
      if (buf[i] == info_sym) {
        sscanf(buf+i+1, "%d", &m[num]);
        num++;
      }
    }
  } while(num < 7);
  FillMarket(market, m);
}

void PrintMarket(InfoMarket &market)
{
  printf("Market: %5d %5d %5d %5d %5d %5d %5d\n", market.lev, market.mnth,
         market.row, market.min_price, market.prod, market.max_price,
         market.cur_cl);
}

void WhoAmI(int sockfd, InfoMarket &market)
{
  const char str[] = "player\n";
  char buf[buf_size];
  int i, len;
  bool chk = false;
  write(sockfd, str, strlen(str));
  do {
    len = read(sockfd, buf, sizeof(buf));
    if (len == 0) {
      throw GameOverResultEx(buf, buf_size, win_sym);
    }
    for (i = 0; (i < len) && (!chk); i++) {
      if (buf[i] == '*') {
        chk = true;
        sscanf(buf+i+1, "%d", &market.me);
      }
    }
  } while(!chk);
}

void FillPlayer(InfoMarket &market, int *m, int indx)
{
  market.players[indx-1].num = indx;
  market.players[indx-1].state = 1;
  market.players[indx-1].mon = m[0];
  market.players[indx-1].row = m[1];
  market.players[indx-1].prod = m[2];
  market.players[indx-1].fact = m[3];
  market.players[indx-1].b_fact = m[4];
}

void ReadPlayers(int sockfd, InfoMarket &market)
{
  int i, j, len, num, m[5];
  char str[str_size], buf[buf_size];
  bool leaved;
  for (i = 1; i <= market.max_cl; i++) {
    sprintf(str, "player %d\n", i);
    write(sockfd, str, strlen(str));
    leaved = false;
    num = 0;
    do {
      len = read(sockfd, buf, sizeof(buf));
      if (len == 0) {
        throw GameOverResultEx(buf, buf_size, win_sym);
      }
      for(j = 0; (j < len) && (!leaved) && (num < 5); j++) {
        if (buf[j] == info_sym) {
         sscanf(buf+j+1, "%d", &m[num]);
         num++;
        }
        if (buf[j] == '^') {
          market.players[i-1].state = 0;
          leaved = true;
        }
      }
    } while((num < 5) && (!leaved));
    if (!leaved) {
      FillPlayer(market, m, i);
    }
  }
}

void PrintPlayers(InfoMarket &market)
{
  int i;
  for (i = 0; i < market.max_cl; i++) {
    if (market.players[i].state == 1) {
      if (i+1 == market.me) {
        printf("You: %11d %5d %5d %5d %5d \n", market.players[i].mon,
                market.players[i].row, market.players[i].prod,
                market.players[i].fact, market.players[i].b_fact);
      } else {
        printf("Player %d: %6d %5d %5d %5d %5d \n", i+1, market.players[i].mon,
                market.players[i].row, market.players[i].prod,
                market.players[i].fact, market.players[i].b_fact);
      }
    }
  }
}

void PrintAuction(InfoMarket &market)
{
  int i;
  InfoBet *bet;
  printf("Auction:\n");
  for (i = 0; (i < market.max_cl) && (market.sold[i].state == 1); i++) {
    bet = &market.sold[i];
    printf("Bank sold %d rows to Player %d for %d\n",
            bet->amnt, bet->num, bet->price);
  }
  for (i = 0; (i < market.max_cl) && (market.bought[i].state == 1); i++) {
    bet = &market.bought[i];
    printf("Bank bought %d products from Player %d for %d\n",
            bet->amnt, bet->num, bet->price);
  }
}

void SetAppl(int sockfd, GameContext &context, const char *instructions_file)
{
  Interpreter program;
  program.Run(instructions_file, context);
}

inline void CreateInfoBet(InfoMarket &market)
{
  market.sold = new InfoBet[market.max_cl];
  market.bought = new InfoBet[market.max_cl];
}

void SetMarketInfo(InfoMarket &market, int sockfd)
{
  WaitStart(sockfd);
  ReadMarket(sockfd, market);
  market.players = new InfoPlayer[market.cur_cl];
  market.max_cl = market.cur_cl;
  CreateInfoBet(market);
  WhoAmI(sockfd, market);
  printf("You are player %d\n", market.me);
}

void DeleteDynamicObjects(InfoMarket &market, ListOfIpnItem *ipn_list)
{
  if (market.players)
    delete[] market.players;
  if (market.sold)
    delete[] market.sold;
  if (market.bought)
    delete[] market.bought;
  if (ipn_list)
    delete ipn_list;
}

int main(int argc, char **argv)
{
  int sockfd;
  InfoMarket market;
  ListOfIpnItem *ipn_list = NULL;
  ListOfVar list_var;
  sockfd = SocketErr();
  GameContext context(&ipn_list, &list_var, market, sockfd);
  Connection(sockfd, argv[1], argv[2]);
  try {
    SetMarketInfo(market, sockfd);
    for(;;) {
      PrintMarket(market);
      ReadPlayers(sockfd, market);
      PrintPlayers(market);
      SetAppl(sockfd, context, argv[3]);
      WaitReadAuction(sockfd, market);
      PrintAuction(market);
      ReadMarket(sockfd, market);
    }
  }
  catch (const GameOverResultEx &e) {
    e.PrintGameOverResultEx();
  }
  DeleteDynamicObjects(market, ipn_list);
  return 0;
}
