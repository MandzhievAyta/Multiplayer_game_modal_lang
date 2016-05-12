#ifndef ROBOT_H_SENTRY
#define ROBOT_H_SENTRY


struct InfoBet {
  int state;
  int num;
  int amnt;
  int price;
};

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
  int min_price;
  int prod;
  int max_price;
  int cur_cl;
  int max_cl;
  int me;
  InfoPlayer *players;
  InfoBet *sold;
  InfoBet *bought;
};

#endif
