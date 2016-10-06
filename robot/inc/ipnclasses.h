#ifndef IPNCLASSES_H_SENTRY
#define IPNCLASSES_H_SENTRY
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "robot.h"
class ListOfVar {
private:
  struct Node {
    char *name;
    int value;
    Node *next;
    Node(): name(NULL), next(NULL) {}
    ~Node() {
      if (name)
        delete[] name;
    }
  };
  Node *first;
  Node *last;
  ListOfVar(const ListOfVar &);
  void operator=(const ListOfVar &);
public:
  ListOfVar(): first(NULL), last(NULL) {}
  ~ListOfVar();
  void Change(const char *varname, int newval);
  int *Get(const char *varname);
};

class ListOfIpnItem;

struct GameContext {
  ListOfIpnItem **ipn_list;
  ListOfVar *list_var;
  InfoMarket &market;
  int sockfd;
  GameContext(ListOfIpnItem **i, ListOfVar *l, InfoMarket &m, int fd)
    : ipn_list(i), list_var(l), market(m), sockfd(fd)
  {}
};

class IpnElemStack;
class IpnItem;

class IpnElem {
public:
  virtual ~IpnElem() {}
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd,
                        GameContext &context) const = 0;
  virtual void Print() = 0;
};

struct IpnItem{
  IpnElem *p;
  IpnItem *next;
  IpnItem(): next(NULL) {}
  ~IpnItem() {}
};

class ListOfIpnItem {
private:
  IpnItem *first;
  IpnItem *last;
  ListOfIpnItem(const ListOfIpnItem &);
  void operator=(const ListOfIpnItem &);
public:
  ListOfIpnItem(): first(NULL), last(NULL) {}
  ~ListOfIpnItem();
  IpnItem *GetAddr() { return last; }
  void Append(IpnElem *object);
  IpnItem *GetFirst() { return first; }
};

class IpnElemStack {
private:
  IpnItem *vertex;
public:
  IpnElemStack(): vertex(NULL) {}
  ~IpnElemStack();
  void Push(IpnElem *elem);
  IpnElem *Pop();
};

class IpnNoOp: public IpnElem {
public:
  IpnNoOp() {}
  virtual ~IpnNoOp() {}
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd,
                        GameContext &context) const
  {
    *cur_cmd = (*cur_cmd)->next;
  }
  virtual void Print()
  {
    printf("IpnNoOp");
  }
};

class IpnConst: public IpnElem {
public:
  virtual IpnElem *Clone() const = 0;
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd,
                        GameContext &context) const
  {
    stack->Push(Clone());
    *cur_cmd = (*cur_cmd)->next;
  }
};

class IpnOpenBracket: public IpnConst {
public:
  IpnOpenBracket() {}
  virtual ~IpnOpenBracket() {}
  virtual IpnElem *Clone() const {
    return new IpnOpenBracket();
  }
  virtual void Print()
  {
    printf("IpnOpenBracket");
  }
};

class IpnInt: public IpnConst {
private:
  int value;
public:
  IpnInt(int a) { value = a; }
  virtual ~IpnInt() {}
  virtual IpnElem *Clone() const {
    return new IpnInt(value);
  }
  int Get() const { return value; }
  virtual void Print()
  {
    printf("IpnInt = %d", value);
  }
};

class IpnString: public IpnConst {
private:
  char *value;
public:
  IpnString(const char *src) {
    value = new char[strlen(src) + 1];
    strcpy(value, src);
  }
  virtual ~IpnString() { delete[] value; }
  virtual IpnElem *Clone() const {
    return new IpnString(value);
  }
  char *Get() const { return value; }
  virtual void Print()
  {
    printf("IpnString = %s", value);
  }
};

class IpnVarAddr: public IpnConst {
private:
  ListOfVar *list;
  char *name;
public:
  IpnVarAddr(ListOfVar *a, const char *b)
  {
    list = a;
    name = new char[strlen(b) + 1];
    strcpy(name, b);
  }
  virtual ~IpnVarAddr() { delete[] name; }
  virtual IpnElem *Clone() const {
    return new IpnVarAddr(list, name);
      }
  void Change(int newvalue) {
     list->Change(name, newvalue);
  }
  int *Get() const { return list->Get(name); }
  void AddDimension(int dim);
  virtual void Print()
  {
    printf("IpnVarAddr = %s", name);
  }
};

class IpnLabel: public IpnConst {
private:
  IpnItem *value;
public:
  IpnLabel(IpnItem *a) { value = a; }
  virtual ~IpnLabel() {}
  virtual IpnElem *Clone() const
  {
    return new IpnLabel(value);
  }
  IpnItem *Get() const { return value; }
  virtual void Print()
  {
    printf("IpnLabel");
  }
};

class IpnPrint: public IpnElem {
public:
  IpnPrint() {}
  virtual ~IpnPrint() {}
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd,
                        GameContext &context) const;
  virtual void Print()
  {
    printf("IpnPrint");
  }
};

class IpnFunction: public IpnElem {
public:
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const = 0;
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd,
                        GameContext &context) const
  {
    IpnElem *res = EvaluateFun(stack, context);
    if (res)
      stack->Push(res);
    *cur_cmd = (*cur_cmd)->next;
  }
};

class IpnSell: public IpnFunction {
public:
  IpnSell() {}
  virtual ~IpnSell() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnSell");
  }
};

class IpnBuy: public IpnFunction {
public:
  IpnBuy() {}
  virtual ~IpnBuy() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnBuy");
  }
};

class IpnProd: public IpnFunction {
public:
  IpnProd() {}
  virtual ~IpnProd() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnProd");
  }
};

class IpnBuild: public IpnFunction {
public:
  IpnBuild() {}
  virtual ~IpnBuild() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnBuild");
  }
};

class IpnMyId: public IpnFunction {
public:
  IpnMyId() {}
  virtual ~IpnMyId() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const
  {
    return new IpnInt(context.market.me);
  }
  virtual void Print()
  {
    printf("IpnMyId");
  }
};

class IpnTurn: public IpnFunction {
public:
  IpnTurn() {}
  virtual ~IpnTurn() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const
  {
    return new IpnInt(context.market.mnth);
  }
  virtual void Print()
  {
    printf("IpnTurn");
  }
};

class IpnPlayers: public IpnFunction {
public:
  IpnPlayers() {}
  virtual ~IpnPlayers() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const
  {
    return new IpnInt(context.market.max_cl);
  }
  virtual void Print()
  {
    printf("IpnPlayers");
  }
};

class IpnActivePlayers: public IpnFunction {
public:
  IpnActivePlayers() {}
  virtual ~IpnActivePlayers() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const
  {
    return new IpnInt(context.market.cur_cl);
  }
  virtual void Print()
  {
    printf("IpnActivePlayers");
  }
};

class IpnSupply: public IpnFunction {
public:
  IpnSupply() {}
  virtual ~IpnSupply() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const
  {
    return new IpnInt(context.market.row);
  }
  virtual void Print()
  {
    printf("IpnSupply");
  }
};

class IpnRawPrice: public IpnFunction {
public:
  IpnRawPrice() {}
  virtual ~IpnRawPrice() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const
  {
    return new IpnInt(context.market.min_price);
  }
  virtual void Print()
  {
    printf("IpnRawPrice");
  }
};

class IpnDemand: public IpnFunction {
public:
  IpnDemand() {}
  virtual ~IpnDemand() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const
  {
    return new IpnInt(context.market.prod);
  }
  virtual void Print()
  {
    printf("IpnDemand");
  }
};

class IpnProductionPrice: public IpnFunction {
public:
  IpnProductionPrice() {}
  virtual ~IpnProductionPrice() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const
  {
    return new IpnInt(context.market.max_price);
  }
  virtual void Print()
  {
    printf("IpnProductionPrice");
  }
};

class IpnMoney: public IpnFunction {
public:
  IpnMoney() {}
  virtual ~IpnMoney() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnMoney");
  }
};

class IpnRaw: public IpnFunction {
public:
  IpnRaw() {}
  virtual ~IpnRaw() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnRaw");
  }
};

class IpnProduction: public IpnFunction {
public:
  IpnProduction() {}
  virtual ~IpnProduction() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnProduction");
  }
};

class IpnFactories: public IpnFunction {
public:
  IpnFactories() {}
  virtual ~IpnFactories() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnFactories");
  }
};

class IpnManufactured: public IpnFunction {
public:
  IpnManufactured() {}
  virtual ~IpnManufactured() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnManufactured");
  }
};

class IpnResultRawSold: public IpnFunction {
public:
  IpnResultRawSold() {}
  virtual ~IpnResultRawSold() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnResultRawSold");
  }
};

class IpnResultRawPrice: public IpnFunction {
public:
  IpnResultRawPrice() {}
  virtual ~IpnResultRawPrice() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnResultRawPrice");
  }
};

class IpnResultProdBought: public IpnFunction {
public:
  IpnResultProdBought() {}
  virtual ~IpnResultProdBought() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnResultProdBought");
  }
};

class IpnResultProdPrice: public IpnFunction {
public:
  IpnResultProdPrice() {}
  virtual ~IpnResultProdPrice() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnResultProdProce");
  }
};

class IpnFunAssign: public IpnFunction {
public:
  IpnFunAssign() {}
  virtual ~IpnFunAssign() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnFunAssign");
  }
};

class IpnFunLogNot: public IpnFunction {
public:
  IpnFunLogNot() {}
  virtual ~IpnFunLogNot() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnFunLogNot");
  }
};

class IpnFunAddDimension: public IpnFunction {
public:
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
  virtual void Print()
  {
    printf("IpnFunAddDimension");
  }
};

class IpnFun2: public IpnFunction {
public:
  virtual int BinaryOp(int, int) const = 0;
  virtual IpnElem *EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const;
};

class IpnFunPlus: public IpnFun2 {
public:
  IpnFunPlus() {}
  virtual ~IpnFunPlus() {}
  virtual int BinaryOp(int i1, int i2) const {
    return i1 + i2;
  }
  virtual void Print()
  {
    printf("IpnFunPlus");
  }
};

class IpnFunMinus: public IpnFun2 {
public:
  IpnFunMinus() {}
  virtual ~IpnFunMinus() {}
  virtual int BinaryOp(int i1, int i2) const {
    return i1 - i2;
  }
  virtual void Print()
  {
    printf("IpnFunMinus");
  }
};

class IpnFunMultiplication: public IpnFun2 {
public:
  IpnFunMultiplication() {}
  virtual ~IpnFunMultiplication() {}
  virtual int BinaryOp(int i1, int i2) const {
    return i1 * i2;
  }
  virtual void Print()
  {
    printf("IpnFunMultiplication");
  }
};

class IpnFunDivision: public IpnFun2 {
public:
  IpnFunDivision() {}
  virtual ~IpnFunDivision() {}
  virtual int BinaryOp(int i1, int i2) const {
    return (int)(i1 / i2);
  }
  virtual void Print()
  {
    printf("IpnFunDivision");
  }
};

class IpnFunLess: public IpnFun2 {
public:
  IpnFunLess() {}
  virtual ~IpnFunLess() {}
  virtual int BinaryOp(int i1, int i2) const {
    if (i1 < i2)
      return 1;
    else
      return 0;
  }
  virtual void Print()
  {
    printf("IpnFunLess");
  }
};

class IpnFunGreater: public IpnFun2 {
public:
  IpnFunGreater() {}
  virtual ~IpnFunGreater() {}
  virtual int BinaryOp(int i1, int i2) const {
    if (i1 > i2)
      return 1;
    else
      return 0;
  }
  virtual void Print()
  {
    printf("IpnFunGreater");
  }
};

class IpnFunEqual: public IpnFun2 {
public:
  IpnFunEqual() {}
  virtual ~IpnFunEqual() {}
  virtual int BinaryOp(int i1, int i2) const {
    if (i1 == i2)
      return 1;
    else
      return 0;
  }
  virtual void Print()
  {
    printf("IpnFunEqual");
  }
};

class IpnFunGreaterEqual: public IpnFun2 {
public:
  IpnFunGreaterEqual() {}
  virtual ~IpnFunGreaterEqual() {}
  virtual int BinaryOp(int i1, int i2) const {
    if (i1 >= i2)
      return 1;
    else
      return 0;
  }
  virtual void Print()
  {
    printf("IpnFunGreaterEqual");
  }
};

class IpnFunLessEqual: public IpnFun2 {
public:
  IpnFunLessEqual() {}
  virtual ~IpnFunLessEqual() {}
  virtual int BinaryOp(int i1, int i2) const {
    if (i1 <= i2)
      return 1;
    else
      return 0;
  }
  virtual void Print()
  {
    printf("IpnFunLessEqual");
  }
};

class IpnFunNotEqual: public IpnFun2 {
public:
  IpnFunNotEqual() {}
  virtual ~IpnFunNotEqual() {}
  virtual int BinaryOp(int i1, int i2) const {
    if (i1 != i2)
      return 1;
    else
      return 0;
  }
  virtual void Print()
  {
    printf("IpnFunNotEqual");
  }
};

class IpnFunLogAnd: public IpnFun2 {
public:
  IpnFunLogAnd() {}
  virtual ~IpnFunLogAnd() {}
  virtual int BinaryOp(int i1, int i2) const {
    if (i1 && i2)
      return 1;
    else
      return 0;
  }
  virtual void Print()
  {
    printf("IpnFunLogAnd");
  }
};

class IpnFunLogOr: public IpnFun2 {
public:
  IpnFunLogOr() {}
  virtual ~IpnFunLogOr() {}
  virtual int BinaryOp(int i1, int i2) const {
    if (i1 || i2)
      return 1;
    else
      return 0;
  }
  virtual void Print()
  {
    printf("IpnFunOr");
  }
};

class IpnTakeValue: public IpnElem {
public:
  IpnTakeValue() {}
  virtual ~IpnTakeValue() {}
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd,
                        GameContext &context) const;
  virtual void Print()
  {
    printf("IpnFunTakeValue");
  }
};

class IpnOpGo: public IpnElem {
public:
  IpnOpGo() {}
  virtual ~IpnOpGo() {}
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd,
                        GameContext &context) const;
  virtual void Print()
  {
    printf("IpnOpGo");
  }
};

class IpnOpGoFalse: public IpnElem {
public:
  IpnOpGoFalse() {}
  virtual ~IpnOpGoFalse() {}
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd,
                        GameContext &context) const;
  virtual void Print()
  {
    printf("IpnOpGoFalse");
  }
};

class IpnEx {
private:
  IpnElem *elem;
public:
  IpnEx(IpnElem *p): elem(p) {}
  virtual void PrintWhichError() const = 0;
  virtual ~IpnEx() {}
  void Print () const
  {
    PrintWhichError();
    elem->Print();
  }
};

class IpnExNotInt: public IpnEx {
public:
  IpnExNotInt(IpnElem *p): IpnEx(p) {}
  virtual ~IpnExNotInt() {}
  virtual void PrintWhichError() const
  {
    printf("Int expected:");
  }
};

class IpnExNotVar: public IpnEx {
public:
  IpnExNotVar(IpnElem *p): IpnEx(p) {}
  virtual ~IpnExNotVar() {}
  virtual void PrintWhichError() const
  {
    printf("Variable expected:");
  }
};

class IpnExVarNotDeclared: public IpnEx {
public:
  IpnExVarNotDeclared(IpnElem *p): IpnEx(p) {}
  virtual ~IpnExVarNotDeclared() {}
  virtual void PrintWhichError() const
  {
    printf("Use of variable before declaration:");
  }
};

class IpnExNotLabel: public IpnEx {
public:
  IpnExNotLabel(IpnElem *p): IpnEx(p) {}
  virtual ~IpnExNotLabel() {}
  virtual void PrintWhichError() const
  {
    printf("Label expected:");
  }
};

#endif
