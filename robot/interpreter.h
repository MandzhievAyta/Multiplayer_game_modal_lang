#ifndef INTERPRETER_H_SENTRY
#define INTERPRETER_H_SENTRY
#include "ipnclasses.h"
#include "robot.h"

class Interpreter {
public:
  void PrintIpn(ListOfIpnItem *ipn_list);
  ListOfIpnItem **CreateIpnList(const char *filename, GameContext &context);
  void Run(const char *filename, GameContext &context);
};
#endif
