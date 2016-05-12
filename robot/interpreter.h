#ifndef INTERPRETER_H_SENTRY
#define INTERPRETER_H_SENTRY
#include "ipnclasses.h"
#include "robot.h"

class Interpreter {
public:
  void
  Run(const char *filename, GameContext &context);
};
#endif
