#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

enum {game_off, game_on};
enum {play, game_over};
enum {random_lev, third_level=3};
enum {descending, ascending};
enum {cl_buf_size=4096};

#define ADDED_CL "\nPlayer %d joined the game(ip %ld)\nWaiting for %d players\n"
#define HELP_PRINT \
"List of commands:\n\
 market        - information about market.\n\
 player arg1   - information about player number arg1.\n\
 prod arg1     - convert arg1 amount of rows to products.\n\
 buy arg1 arg2 - buy arg1 amount of rows for arg2 $.\n\
 sell arg1 arg2- sell arg1 amount of products for arg2 $.\n\
 build arg1    - build arg1 amount of factories.\n\
 turn          - finish your turn.\n\
 help          - information about commands.\n"

struct sell_buy {
  int amnt;
  int price;
  struct list_clients *cl;
  int state;
};

struct build_fact {
  int rest_days;
  struct build_fact *next;
};

struct banker {
  int state;
  int max_cl;
  int cur_cl;
  int mnth;
  int row;
  int prod;
  int min_p;
  int max_p;
  int lev;
  struct sell_buy *sell;
  struct sell_buy *buy;
  struct list_clients *cl;
};

struct list_clients {
  int num;
  int fd;
  int state;
  char buf[cl_buf_size];
  int cur_size;
  unsigned long ip;
  int mon;
  int row;
  int prod;
  int fact;
  int conv_row;
  struct build_fact *b_fact;
  int turn;
  struct list_clients *next;
};

struct list_clients *delete_client(struct banker *bank, int num_del);
void check_winner(struct banker *bank);

int socket_err(int *err)
{
  int ls;
  ls=socket(AF_INET, SOCK_STREAM, 0);
  if (ls==-1) {
    perror("socket");
    *err=1;
  }
  return ls;
}

void bind_err(struct sockaddr_in *addr, int ls, int *err)
{
  int help;
  if (!*err) help=bind(ls, (struct sockaddr*) addr, sizeof(*addr));
  if ((0 != help) && (!*err)) {
    perror("bind");
    *err=1;
  }
}

void listen_err(int ls, int *err)
{
  int help;
  if (!*err) help=listen(ls,5);
  if ((0 != help) && (!*err)) {
    perror("listen");
    *err=1;
  }
}

void select_err(fd_set *readfds, int max_d)
{
  int res_select;
  res_select=select(max_d+1, readfds, NULL, NULL, NULL);
  if (res_select<1) {
    perror("select");
  }
}

void write_to_all(struct banker *bank,const char *string)
{
  struct list_clients *cl=bank->cl;
  while (cl) {
    write(cl->fd, string, strlen(string));
    cl=cl->next;
  }
}

void write_inv_to_all(struct banker *bank)
{
  char string[20];
  struct list_clients *cl=bank->cl;
  while (cl) {
    sprintf(string, "Player %d:", cl->num);
    write(cl->fd, string, strlen(string));
    cl=cl->next;
  }
}

void write_inv(int fd, int which_client)
{
  char string[20];
  sprintf(string, "Player %d:", which_client);
  write(fd, string, strlen(string));
}

void appl_accept(struct list_clients *cl)
{
  const char string[]="Application accepted!\n";
  write(cl->fd, string, strlen(string));
}

void end_of_turn(int fd, int which_client)
{
  const char string[]="You finished your turn!\n";
  write(fd, string, strlen(string));
  write_inv(fd, which_client);
}

void wrng_cmd(struct list_clients *cl)
{
  const char buf[]=
    "You entered wrong command! Type help to watch the list!\n";
  write(cl->fd, buf, strlen(buf));
  write_inv(cl->fd, cl->num);
}

void discon_cl(int fd)
{
  shutdown(fd, 2);
  close(fd);
}

void delete_all_clients(struct banker *bank)
{
  struct list_clients **p=&(bank->cl);
  while (*p) {
    (*p)=delete_client(bank, (**p).num);
  }
}

void not_started(struct list_clients *cl)
{
  const char string[]="The game has not started yet\n";
  write(cl->fd, string, strlen(string));
  write_inv(cl->fd, cl->num);
}

void started(struct banker *bank)
{
  const char string[]="\nThe game started!\n";
  bank->state=game_on;
  write_to_all(bank, string);
  write_inv_to_all(bank);
}

int act_pl(struct banker *bank)
{
  int i=0;
  struct list_clients *cl=bank->cl;
  while (cl) {
    if (cl->state==play) i++;
    cl=cl->next;
  }
  return i;
}

void delete_b_fact(struct list_clients *cl)
{
  struct build_fact *first=cl->b_fact, *cur;
  while (first) {
    cur=first->next;
    free(first);
    first=cur;
  }
  cl->b_fact=NULL;
}

int generate_new_level(struct banker *bank)
{
  int r, i=0, cum=0;
  const int level_change[5][5]={
    {4, 4, 2, 1, 1},
    {3, 4, 3, 1, 1},
    {1, 3, 4, 3, 1},
    {1, 1, 3, 4, 3},
    {1, 1, 2, 4, 4}
  };

  r = 1 + (int)(12.0*rand()/(RAND_MAX+1.0));
  while (r>cum) {
    cum=cum+level_change[bank->lev][i];
    i++;
  }
  return i;
}

void change_level(struct banker *bank, int arg)
{
  int amnt_pl=act_pl(bank);
  if (arg==random_lev) bank->lev=generate_new_level(bank);
  else {
    bank->lev=third_level;
    amnt_pl=bank->max_cl;
  }
  bank->mnth++;
  switch (bank->lev) {
    case 1:
      bank->min_p=800;
      bank->row=amnt_pl;
      bank->max_p=6500;
      bank->prod=3*amnt_pl;
      break;
    case 2:
      bank->min_p=650;
      bank->row=(int)1.5*amnt_pl;
      bank->max_p=6000;
      bank->prod=(int)2.5*amnt_pl;
      break;
    case 3:
      bank->min_p=500;
      bank->row=2*amnt_pl;
      bank->max_p=5500;
      bank->prod=2*amnt_pl;
      break;
    case 4:
      bank->min_p=400;
      bank->row=(int)2.5*amnt_pl;
      bank->max_p=5000;
      bank->prod=(int)1.5*amnt_pl;
      break;
    case 5:
      bank->min_p=300;
      bank->row=3*amnt_pl;
      bank->max_p=4500;
      bank->prod=amnt_pl;
      break;
  }
}

void null_sell_buy(struct banker *bank)
{
  int i;
  for (i=0;i<bank->max_cl;i++) {
    (*bank).sell[i].state=0;
    (*bank).buy[i].state=0;
  }
}

void write_winner(struct banker *bank, struct list_clients *cl)
{
  char string[30];
  if (cl) sprintf(string, "\nPlayer %d won!\n", cl->num);
  else sprintf(string, "\nAll players became bankrupts\n");
  write_to_all(bank, string);
}

void write_wait_cl(struct banker *bank, unsigned long ip)
{
  char string[100];
  int how_many=bank->max_cl-bank->cur_cl;
  sprintf(string, ADDED_CL, bank->cur_cl, ip, how_many);
  write_to_all(bank, string);
  write_inv_to_all(bank);
}

void set_sockaddr(struct sockaddr_in *addr, char **argv)
{
  addr->sin_family = AF_INET;
  addr->sin_port = htons(atoi(argv[1]));
  addr->sin_addr.s_addr = INADDR_ANY;
}

int fill_set(struct list_clients *clients, fd_set *readfds, int ls)
{
  int max_d;
  FD_ZERO(readfds);
  FD_SET(ls, readfds);
  max_d=ls;
  while (clients) {
    FD_SET(clients->fd, readfds);
    if (clients->fd>max_d) {
      max_d=clients->fd;
    }
    clients=clients->next;
  }
  return max_d;
}

void write_bought(struct banker *bank, struct sell_buy sell, int amnt)
{
  char string[100];
  struct list_clients *all_cl=bank->cl;
  sprintf(string, "Bank sold %d rows to Player %d for %d\n",
          amnt, sell.cl->num, sell.price);
  while (all_cl) {
    write(all_cl->fd, string, strlen(string));
    all_cl=all_cl->next;
  }
}

void new_game(struct banker *bank)
{
  null_sell_buy(bank);
  bank->mnth=0;
  change_level(bank, third_level);
  bank->state=game_off;
}

void check_winner(struct banker *bank)
{
  int i=0;
  struct list_clients *cl=bank->cl;
  while (cl) {
    if (cl->state==play) i++;
    cl=cl->next;
  }
  cl=bank->cl;
  if (i==1) {
    while (cl->state!=play){
      cl=cl->next;
    }
  } else
  if (i==0) cl=NULL;
  if ((i==1 || i==0) && (bank->state==game_on)) {
    bank->state=game_off;
    write_winner(bank, cl);
    new_game(bank);
    delete_all_clients(bank);
  }
}

void write_sold(struct banker *bank, struct sell_buy buy, int amnt)
{
  char string[100];
  struct list_clients *all_cl=bank->cl;
  sprintf(string, "Bank bought %d products from Player %d for %d\n",
          amnt, buy.cl->num, buy.price);
  while (all_cl) {
    write(all_cl->fd, string, strlen(string));
    all_cl=all_cl->next;
  }
}

void extra_client(int new_fd)
{
  const char string[]="The game has already started\n";
  write(new_fd, string, strlen(string));
  discon_cl(new_fd);
}

void add_client(struct banker *bank,
                struct sockaddr_in *addr, int new_fd)
{
  struct list_clients **p=&(bank->cl);
  while (*p) {
    p=&((**p).next);
  }
  (*p)=malloc(sizeof(**p));
  (**p).ip=(*addr).sin_addr.s_addr;
  (**p).fd=new_fd;
  (**p).next=NULL;
  (**p).cur_size=0;
  (**p).num=bank->cur_cl;
  (**p).state=play;
  (**p).mon=10000;
  (**p).row=4;
  (**p).prod=2;
  (**p).fact=2;
  (**p).turn=1;
  (**p).conv_row=0;
  (**p).b_fact=NULL;
  write_wait_cl(bank, (**p).ip);
  if (bank->cur_cl==bank->max_cl) started(bank);
}

void accept_client(struct banker *bank, fd_set *readfds, int ls)
{
  struct sockaddr_in addr;
  int new_fd;
  unsigned int addrlen;
  addrlen=sizeof(addr);
  if (FD_ISSET(ls, readfds)) {
    new_fd=accept(ls, (struct sockaddr*) &addr, &addrlen);
    if (-1 == new_fd) {
      perror("accept");
    } else
    if (bank->state==game_off) {
      bank->cur_cl++;
      add_client(bank, &addr, new_fd);
    } else {
      extra_client(new_fd);
    }
  }
}
/*
void movestr(char *string, int amnt)
{
  int i=0;
  while (string[i]!='\0') {
    string[i]=string[i+amnt];
    i++;
  }
}

void delete_extra_spaces(struct list_clients *cl)
{
  int i=0;
  while (cl->buf[0]==' ') {
    movestr(cl->buf, 1);
  }
  while (cl->buf[i]!='\0') {
    if ((cl->buf[i]==' ') && (cl->buf[i+1]==' ')) {
      movestr(cl->buf+i, 1);
    } else {
      i++;
    }
  }
  if (cl->buf[i-1]==' ') cl->buf[i-1]='\0';
}
*/
void market(struct list_clients *cl, struct banker *bank)
{
  char string[100];
  sprintf(string, "Current market level:\n#%d\n", bank->lev);
  write(cl->fd, string, strlen(string));
  sprintf(string, "Current month:\n#%d\n", bank->mnth);
  write(cl->fd, string, strlen(string));
  sprintf(string, "The amount of sold raw:\n#%d\n", bank->row);
  write(cl->fd, string, strlen(string));
  sprintf(string, "The minimum price per row:\n#%d\n", bank->min_p);
  write(cl->fd, string, strlen(string));
  sprintf(string, "The amount of purchased product:\n#%d\n", bank->prod);
  write(cl->fd, string, strlen(string));
  sprintf(string, "The maximum price per product:\n#%d\n", bank->max_p);
  write(cl->fd, string, strlen(string));
  sprintf(string, "The number of active players:\n#%d\n", act_pl(bank));
  write(cl->fd, string, strlen(string));
  write_inv(cl->fd, cl->num);
}

int amnt_b(struct list_clients *cl)
{
  int i=0;
  struct build_fact *fact=cl->b_fact;
  while (fact) {
    i++;
    fact=fact->next;
  }
  return i;
}

void player(struct list_clients *cl, int arg1, struct banker *bank)
{
  struct list_clients *inf_cl=bank->cl;
  char string[100];
  if (arg1==0) 
    arg1=cl->num;
  while (inf_cl->num!=arg1) {
    inf_cl=inf_cl->next;
    if (!inf_cl)
      break;
  }
  if (inf_cl) {
    sprintf(string, "Information about player %d:\n", arg1);
    write(cl->fd, string, strlen(string));
    sprintf(string, "Amount of money:\n#%d\n", inf_cl->mon);
    write(cl->fd, string, strlen(string));
    sprintf(string, "Amount of row:\n#%d\n", inf_cl->row);
    write(cl->fd, string, strlen(string));
    sprintf(string, "Amount of products:\n#%d\n", inf_cl->prod);
    write(cl->fd, string, strlen(string));
    sprintf(string, "Amount of factories:\n#%d\n", inf_cl->fact);
    write(cl->fd, string, strlen(string));
    sprintf(string, "Amount of factories under constructions:\n#%d\n",
            amnt_b(inf_cl));
    write(cl->fd, string, strlen(string));
    sprintf(string, "Did player finish his turn?:\n");
    write(cl->fd, string, strlen(string));
    if (inf_cl->turn) sprintf(string, "No\n");
    else sprintf(string, "Yes\n");
    write(cl->fd, string, strlen(string));
  } else {
    sprintf(string, "Can not find this player!\n");
    write(cl->fd, string, strlen(string));
  }
  write_inv(cl->fd, cl->num);
}

void prod(struct list_clients *cl, int arg1)
{
  char string[100];
  if (arg1==0) arg1=1;
  if ((cl->conv_row+arg1<=cl->fact) &&
      (arg1<=cl->row) && (arg1>=0) &&
      (arg1*2000<=cl->mon)) {
    cl->conv_row=cl->conv_row+arg1;
    cl->mon=cl->mon-arg1*2000;
    cl->row=cl->row-arg1;
    cl->prod=cl->prod+arg1;
    appl_accept(cl);
  } else {
    if (cl->conv_row+arg1>cl->fact) {
      sprintf(string, "You do not have enough factories\n");
      write(cl->fd, string, strlen(string));
    }
    if (arg1>cl->row) {
      sprintf(string, "You do not have enough rows\n");
      write(cl->fd, string, strlen(string));
    }
    if (arg1*2000>cl->mon) {
      sprintf(string, "You do not have enough money\n");
      write(cl->fd, string, strlen(string));
    }
    if (arg1<0) {
      sprintf(string, "You entered wrong amount of products!\n");
      write(cl->fd, string, strlen(string));
    }
  }
  write_inv(cl->fd, cl->num);
}

void buy(struct list_clients *cl, int arg1, int arg2, struct banker *bank)
{
  char string[100];
  int i=0;
  if ((arg1<=bank->row) && (arg2*arg1<=cl->mon) && 
      (arg2>=bank->min_p) && (arg1>=0) && (arg2>=0)) {
    while ((*bank).buy[i].state==1) {
      if ((*bank).buy[i].cl==cl) break;
      i++;
    }
    (*bank).buy[i].state=1;
    (*bank).buy[i].cl=cl;
    (*bank).buy[i].amnt=arg1;
    (*bank).buy[i].price=arg2;
    appl_accept(cl);
  } else {
    if ((arg1<0) || (arg2<0)) {
      sprintf(string, "You entered bad request\n");
      write(cl->fd, string, strlen(string));
    }
    if (arg1>bank->row) {
      sprintf(string, "Bank does not have %d rows\n", arg1);
      write(cl->fd, string, strlen(string));
    }
    if (arg2*arg1>cl->mon) {
      sprintf(string, "You do not have enough money!\n");
      write(cl->fd, string, strlen(string));
    }
    if (arg2<bank->min_p) {
      sprintf(string, "Too low price!\n");
      write(cl->fd, string, strlen(string));
    }
  }
  write_inv(cl->fd, cl->num);
}

void sell(struct list_clients *cl, int arg1, int arg2, struct banker *bank)
{
  char string[100];
  int i=0;
  if ((arg1<=cl->prod) && (arg1<=bank->prod) && 
    (arg2<=bank->max_p) && (arg1>=0) && (arg2>=0)) {
    while ((*bank).sell[i].state==1) {
      if ((*bank).sell[i].cl==cl) break;
      i++;
    }
    (*bank).sell[i].state=1;
    (*bank).sell[i].cl=cl;
    (*bank).sell[i].amnt=arg1;
    (*bank).sell[i].price=arg2;
    appl_accept(cl);
  } else {
    if ((arg1<0) || (arg2<0)) {
      sprintf(string, "You entered bad request\n");
      write(cl->fd, string, strlen(string));
    }
    if (arg1>bank->prod) {
      sprintf(string, "Bank will not buy %d products\n", arg1);
      write(cl->fd, string, strlen(string));
    }
    if (arg1>cl->prod) {
      sprintf(string, "You do not have enough products!\n");
      write(cl->fd, string, strlen(string));
    }
    if (arg2>bank->max_p) {
      sprintf(string, "Too high price!\n");
      write(cl->fd, string, strlen(string));
    }
  }
  write_inv(cl->fd, cl->num);
}

void build(struct list_clients *cl, int arg1)
{
  char string[100];
  struct build_fact **p=&(cl->b_fact);
  if (arg1==0) arg1=1;
  if ((arg1*2500<=cl->mon) && (arg1>=0)) {
    while (*p) p=&((**p).next);
    cl->mon=cl->mon-2500*arg1;
    while (arg1>0) {
      (*p)=malloc(sizeof(**p));
      (**p).rest_days=5;
      (**p).next=NULL;
      p=&((**p).next);
      arg1--;
    }
    appl_accept(cl);
  } else {
    if (arg1<0) {
      sprintf(string, "You entered bad request\n");
      write(cl->fd, string, strlen(string));
    }
    if (arg1*2500<=cl->mon) {
      sprintf(string, "You do not have enough money!\n");
      write(cl->fd, string, strlen(string));
    }
  }
  write_inv(cl->fd, cl->num);
}

void help(struct list_clients *cl)
{
  const char *string=HELP_PRINT;
  write(cl->fd, string, strlen(string));
  write_inv(cl->fd, cl->num);
}

void turn(struct list_clients *cl)
{
  cl->turn=0;
  write_inv(cl->fd, cl->num);
}

void change_value(struct list_clients *cl, struct banker *bank)
{
  char cmd[sizeof(cl->buf)];
  int em, arg1, arg2;
  em=sscanf(cl->buf, "%s %d %d", cmd, &arg1, &arg2);
  if (em==1) arg1=0;
  if ((!strcmp(cmd, "market")) && (em==1))
    market(cl, bank);
  else if ((!strcmp(cmd, "player")) && (em==2 || em==1))
    player(cl, arg1, bank);
  else if ((!strcmp(cmd, "prod")) && (em==2 || em==1) && (cl->turn))
    prod(cl, arg1);
  else if ((!strcmp(cmd, "buy")) && (em==3) && (cl->turn))
    buy(cl, arg1, arg2, bank);
  else if ((!strcmp(cmd, "sell")) && (em==3) && (cl->turn))
    sell(cl, arg1, arg2, bank);
  else if ((!strcmp(cmd, "build")) && (em==2 || em==1) && (cl->turn))
    build(cl, arg1);
  else if ((!strcmp(cmd, "turn")) && (em==1) && (cl->turn))
    turn(cl);
  else if ((!strcmp(cmd, "help")) && (em==1))
    help(cl);
  else if (cl->turn)
    wrng_cmd(cl);
  else
    end_of_turn(cl->fd, cl->num);
}

struct list_clients *delete_client(struct banker *bank, int num_del)
{
  char string[30];
  struct list_clients *del_cl=bank->cl, **prev_cl=&(bank->cl);
  while (del_cl->num!=num_del) {
    prev_cl=&(del_cl->next);
    del_cl=del_cl->next;
  }
  *prev_cl=(**prev_cl).next;
  discon_cl(del_cl->fd);
  free(del_cl);
  sprintf(string, "\nPlayer %d left the game!\n", num_del);
  write_to_all(bank, string);
  bank->cur_cl--;
  return *prev_cl;
}

void handle_clients(struct banker *bank, fd_set *readfds)
{
  int len, last_sym, del=0;
  struct list_clients *cl=bank->cl;
  while (cl) {
    if (FD_ISSET(cl->fd, readfds)) {
      len=read(cl->fd, cl->buf+cl->cur_size, sizeof(cl->buf)-cl->cur_size);
      cl->cur_size+=len-2;
      if (cl->cur_size==sizeof(cl->buf)) {
        cl->cur_size=0;
      }
      last_sym=cl->cur_size;
      if (cl->buf[last_sym+1]==0) {
        cl=delete_client(bank, cl->num);
        write_inv_to_all(bank);
        check_winner(bank);
        del=1;
      } else
      if (cl->buf[last_sym+1]=='\n') {
        cl->buf[last_sym]='\0';
        if (bank->state==game_on) change_value(cl, bank);
        else not_started(cl);
        cl->cur_size=0;
      }
    }
    if (!del) cl=cl->next;
    del=0;
  }
}

void bank_null(struct banker *bank, char **argv)
{
  bank->cl=NULL;
  bank->max_cl=atoi(argv[2]);
  bank->state=game_off;
  bank->mnth=0;
  change_level(bank, third_level);
  bank->cur_cl=0;
  bank->row=bank->max_cl*2;
  bank->prod=bank->max_cl*2;
  bank->sell=malloc(sizeof(struct sell_buy)*bank->max_cl);
  bank->buy=malloc(sizeof(struct sell_buy)*bank->max_cl);
  null_sell_buy(bank);
}

void bubble_sort(struct sell_buy *arr, int max, int mode)
{
  int len=0, i, j, flag=1;
  struct sell_buy tmp;
  while (arr[len].state==1) {
    len++;
    if (len==max) break;
  }
  for(i=1; (i<=len) && flag; i++) {
    flag=0;
    for (j=0; j<(len-1); j++) {
      if ((mode==descending)?(arr[j+1].price>arr[j].price):
                             (arr[j+1].price<arr[j].price)) 
      {
        tmp=arr[j];
        arr[j]=arr[j+1];
        arr[j+1]=tmp;
        flag=1;
      }
    }
  }
}

void print_arr(struct banker *bank)
{
  char string[100];
  int i;
  struct list_clients *cl=bank->cl;
  while (cl) {
    i=0;
    while (bank->sell[i].state==1) {
      sprintf(string ,"Player %d wants to sell %d products for %d\n",
              (*bank).sell[i].cl->num, bank->sell[i].amnt, bank->sell[i].price);
      write(cl->fd, string, strlen(string));
      if (i==bank->max_cl) break;
      i++;
    }
    i=0;
    while (bank->buy[i].state==1) {
      sprintf(string, "Player %d wants to buy %d rows for %d\n",
              (*bank).buy[i].cl->num, bank->buy[i].amnt, bank->buy[i].price);
      write(cl->fd, string, strlen(string));
      if (i==bank->max_cl) break;
      i++;
    }
  cl=cl->next;
  }
}

int chk_month(struct banker *bank)
{
  int i=0;
  struct list_clients *cl=bank->cl;
  while (cl) {
    if ((cl->turn==1) && (cl->state==play)) i++;
    cl=cl->next;
  }
  return !i;
}

void chk_fact(struct banker *bank)
{
  struct list_clients *cl=bank->cl;
  struct build_fact *fact, **p;
  while (cl) {
    p=&(cl->b_fact);
    while (*p) {
      (*p)->rest_days--;
      if ((*p)->rest_days==0) {
        cl->mon=cl->mon-2500;
        cl->fact++;
        fact=(*p);
        (*p)=(**p).next;
        free(fact);
      } else break;
    }
    fact=cl->b_fact;
    while (fact) {
      fact=fact->next;
      if (fact) fact->rest_days--;
    }
    cl=cl->next;
  }
}

void not_enough_prod(struct banker *bank, int f_cl, int l_cl)
{
  struct sell_buy *sell=bank->sell;
  int r, min_p=sell[f_cl].price;
  while (bank->prod) {
    do {
     r=f_cl+(int)((float)(l_cl-f_cl+1)*rand()/(RAND_MAX+1.0));
    } while(sell[r].state==0);
    if (bank->prod>sell[r].amnt) {
      sell[r].cl->mon=sell[r].cl->mon+min_p*sell[r].amnt;
      sell[r].cl->prod=sell[r].cl->prod-sell[r].amnt;
      sell[r].state=0;
      bank->prod=bank->prod-sell[r].amnt;
      write_sold(bank, sell[r], sell[r].amnt);
    } else {
      sell[r].cl->mon=sell[r].cl->mon+min_p*bank->prod;
      sell[r].cl->prod=sell[r].cl->prod-bank->prod;
      write_sold(bank, sell[r], bank->prod);
      bank->prod=0;
      break;
    }
  }
}

void handle_sell(struct banker *bank)
{
  struct sell_buy *sell=bank->sell;
  int min_p, l_cl=0, j, f_cl, amnt=0;
  while ((sell[l_cl].state) && (bank->prod>0)) {
    f_cl=l_cl;
    min_p=sell[l_cl].price;
    amnt=amnt+sell[l_cl].amnt;
    if (l_cl+1!=bank->max_cl)
      while (sell[l_cl+1].state==1) {
        if (sell[l_cl+1].price!=min_p) break;
        amnt=amnt+sell[l_cl+1].amnt;
        if (l_cl+1==bank->max_cl) break;
        l_cl++;
      }
    if (amnt<=bank->prod) {
      for(j=f_cl; j <= l_cl; j++) {
        sell[j].cl->mon=sell[j].cl->mon+min_p*sell[j].amnt;
        sell[j].cl->prod=sell[j].cl->prod-sell[j].amnt;
        bank->prod=bank->prod-sell[j].amnt;
        write_sold(bank, sell[j], sell[j].amnt);
      }
    } else not_enough_prod(bank, f_cl, l_cl);
    l_cl++;
    if (l_cl==bank->max_cl) break;
    amnt=0;
  }
}

void not_enough_row(struct banker *bank, int f_cl, int l_cl)
{
  struct sell_buy *buy=bank->buy;
  int r, max_p=buy[f_cl].price;
  while (bank->row) {
    do {
     r=f_cl+(int)((float)(l_cl-f_cl+1)*rand()/(RAND_MAX+1.0));
    } while(buy[r].state==0);
    if (bank->row>buy[r].amnt) {
      buy[r].cl->mon=buy[r].cl->mon-max_p*buy[r].amnt;
      buy[r].cl->row=buy[r].cl->row+buy[r].amnt;
      buy[r].state=0;
      bank->row=bank->row-buy[r].amnt;
      write_bought(bank, buy[r], buy[r].amnt);
    } else {
      buy[r].cl->mon=buy[r].cl->mon-max_p*bank->row;
      buy[r].cl->row=buy[r].cl->row+bank->row;
      write_bought(bank, buy[r], bank->row);
      bank->row=0;
      break;
    }
  }
}

void handle_buy(struct banker *bank)
{
  struct sell_buy *buy=bank->buy;
  int max_p, l_cl=0, j, f_cl, amnt=0;
  while ((buy[l_cl].state) && (bank->row>0)) {
    f_cl=l_cl;
    max_p=buy[l_cl].price;
    amnt=amnt+buy[l_cl].amnt;
    if (l_cl+1!=bank->max_cl)
      while (buy[l_cl+1].state==1) {
        if (buy[l_cl+1].price!=max_p) break;
        amnt=amnt+buy[l_cl+1].amnt;
        if (l_cl+1==bank->max_cl) break;
        l_cl++;
      }
    if (amnt<=bank->row) {
      for(j=f_cl; j <= l_cl; j++) {
        buy[j].cl->mon=buy[j].cl->mon-max_p*buy[j].amnt;
        buy[j].cl->row=buy[j].cl->row+buy[j].amnt;
        bank->row=bank->row-buy[j].amnt;
        write_bought(bank, buy[j], buy[j].amnt);
      }
    } else not_enough_row(bank, f_cl, l_cl);
    l_cl++;
    if (l_cl==bank->max_cl) break;
    amnt=0;
  }
}

void chk_comis(struct banker *bank)
{
  char string[50];
  struct list_clients *cl=bank->cl;
  while (cl) {
    cl->mon=cl->mon-300*cl->row;
    cl->mon=cl->mon-500*cl->prod;
    cl->mon=cl->mon-1000*cl->fact;
    cl->turn=1;
    if (cl->mon<=0) {
      cl->state=game_over;
      sprintf(string, "Player %d became a bankrupt!\n", cl->num);
      write_to_all(bank, string);
      cl=delete_client(bank, cl->num);
    } else cl=cl->next;
  }
  check_winner(bank);
}

void auction_started(struct banker *bank)
{
  const char string[]="\nAuction started!\n";
  struct list_clients *cl=bank->cl;
  while (cl) {
    write(cl->fd, string, sizeof(string)-1);
    cl=cl->next;
  }
}

void auction(struct banker *bank)
{
  struct list_clients *cl;
  bubble_sort(bank->sell, bank->max_cl, ascending);
  bubble_sort(bank->buy, bank->max_cl, descending);
  chk_fact(bank);
  auction_started(bank);
  print_arr(bank);
  handle_buy(bank);
  handle_sell(bank);
  null_sell_buy(bank);
  chk_comis(bank);
  change_level(bank, random_lev);
  cl=bank->cl;
  while (cl) {
    write_inv(cl->fd, cl->num);
    cl->conv_row=0;
    cl=cl->next;
  }
}

int main(int argc, char **argv)
{
  int ls, max_d, opt=1, err=0;
  fd_set readfds;
  struct sockaddr_in addr;
  struct banker bank;
  bank_null(&bank,argv);
  ls=socket_err(&err);
  if (!err) setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  set_sockaddr(&addr, argv);
  bind_err(&addr, ls, &err);
  listen_err(ls, &err);
  if (!err) {
    for (;;) {
      max_d=fill_set(bank.cl, &readfds, ls);
      select_err(&readfds, max_d);
      accept_client(&bank, &readfds, ls);
      handle_clients(&bank, &readfds);
      if (chk_month(&bank)) auction(&bank);
    }
  }
  return 0;
}
