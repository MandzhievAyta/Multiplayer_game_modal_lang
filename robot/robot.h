#ifndef ROBOT_H_SENTRY
#define ROBOT_H_SENTRY


struct InfoBet {
  int state;
  int num;
  int amnt;
  int price;
  InfoBet(): state(0), num(0), amnt(0), price(0) {}
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
  InfoMarket()
    :lev(0), mnth(0), row(0), min_price(0), prod(0), max_price(0), cur_cl(0),
    max_cl(0), me(0), players(NULL), sold(NULL), bought(NULL)
  {}
};

#endif
