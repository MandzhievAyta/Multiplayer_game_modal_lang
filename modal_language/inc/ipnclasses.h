#ifndef IPNCLASSES_H_SENTRY
#define IPNCLASSES_H_SENTRY
#include <stddef.h>
#include <stdio.h>
#include <string.h>
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

class IpnElemStack;
class IpnItem;

class IpnElem {
public:
  virtual ~IpnElem() {}
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd) const = 0;
};

struct IpnItem{
  IpnElem *p;
  IpnItem *next;
  IpnItem(): next(NULL) {}
};

class ListOfIpnItem {
private:
  IpnItem *first;
  IpnItem *last;
/*  IpnItem *cur;*/
  ListOfIpnItem(const ListOfIpnItem &);
  void operator=(const ListOfIpnItem &);
public:
  ListOfIpnItem(): first(NULL), last(NULL) {}
  ~ListOfIpnItem();
  IpnItem *GetAddr() { return last; }
  void Append(IpnElem *object);
/*  void StartIter() { cur = first; }*/
/*  IpnElem *GetNext();*/
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
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd) const {
    *cur_cmd = (*cur_cmd)->next;
  }
};

class IpnConst: public IpnElem {
public:
  virtual IpnElem *Clone() const = 0;
  void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd) const
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
  void AddDimension(int dim) {
    char string[1024], *tmp = new char[strlen(name)];
    sprintf(string, "%d", dim);
    tmp = new char[strlen(name) + strlen(string) + 3];
    sprintf(tmp, "%s[%d]", name, dim);
    delete[] name;
    name = tmp;
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
};

class IpnPrint: public IpnElem {
public:
  IpnPrint() {}
  virtual ~IpnPrint() {}
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd) const {
    IpnElemStack local_stack;
    IpnElem *operand1 = stack->Pop();
    IpnInt *ipn_int;
    IpnString *ipn_str;
    IpnOpenBracket *ipn_brkt;
    while (!(ipn_brkt = dynamic_cast<IpnOpenBracket*>(operand1))) {
      local_stack.Push(operand1);
      operand1 = stack->Pop();
    }
    delete operand1;
    operand1 = local_stack.Pop();
    while (operand1) {
      ipn_int = dynamic_cast<IpnInt*>(operand1);
      if (ipn_int) {
        printf("%d", ipn_int->Get());
      } else {
        ipn_str = dynamic_cast<IpnString*>(operand1);
        printf("%s", ipn_str->Get());
      }
      delete operand1;
      operand1 = local_stack.Pop();
    }
    printf("\n");
    *cur_cmd = (*cur_cmd)->next;
  }
};

class IpnFunction: public IpnElem {
public:
  virtual IpnElem *EvaluateFun(IpnElemStack *stack) const = 0;
  void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd) const
  {
    IpnElem *res = EvaluateFun(stack);
    if (res)
      stack->Push(res);
    *cur_cmd = (*cur_cmd)->next;
  }
};

class IpnSell: public IpnFunction {
public:
  IpnSell() {}
  virtual ~IpnSell() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack) const {
    return NULL;
  }
};

class IpnBuy: public IpnFunction {
public:
  IpnBuy() {}
  virtual ~IpnBuy() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack) const {
    return NULL;
  }
};

class IpnProd: public IpnFunction {
public:
  IpnProd() {}
  virtual ~IpnProd() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack) const {
    return NULL;
  }
};

class IpnBuild: public IpnFunction {
public:
  IpnBuild() {}
  virtual ~IpnBuild() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack) const {
    return NULL;
  }
};

class IpnEndTurn: public IpnFunction {
public:
  IpnEndTurn() {}
  virtual ~IpnEndTurn() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack) const {
    return NULL;
  }
};

class IpnFunAssign: public IpnFunction {
public:
  IpnFunAssign() {}
  virtual ~IpnFunAssign() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack) const
  {
    IpnElem *operand1 = stack->Pop();
    IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
/*    if (!i1)
      throw IpnExNotInt(operand1);*/
    IpnElem *operand2 = stack->Pop();
    IpnVarAddr *i2 = dynamic_cast<IpnVarAddr*>(operand2);
/*    if (!i2)
      throw IpnExNotVar(operand2);*/
    i2->Change(i1->Get());
    delete operand1;
    delete operand2;
    return NULL;
  }
};

class IpnFunLogNot: public IpnFunction {
public:
  IpnFunLogNot() {}
  virtual ~IpnFunLogNot() {}
  virtual IpnElem *EvaluateFun(IpnElemStack *stack) const
  {
    IpnElem *operand = stack->Pop();
    IpnInt *i1 = dynamic_cast<IpnInt*>(operand);
/*    if (!i1)
      throw IpnExNotLogNot(operand1);*/
    int res = !(i1->Get());
    return new IpnInt(res);
  }
};

class IpnFunAddDimension: public IpnFunction {
public:
  virtual IpnElem *EvaluateFun(IpnElemStack *stack) const
  {
    IpnElem *operand1 = stack->Pop();
    IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
/*    if (!i1)
      throw IpnExNotInt(operand1);*/
    IpnElem *operand2 = stack->Pop();
    IpnVarAddr *i2 = dynamic_cast<IpnVarAddr*>(operand2);
/*    if (!i2)
      throw IpnExNotVarAddr(operand1);*/
    i2->AddDimension(i1->Get());
    delete operand1;
    return i2;
  }
};

class IpnFun2: public IpnFunction {
public:
  virtual int BinaryOp(int, int) const = 0;
  virtual IpnElem *EvaluateFun(IpnElemStack *stack) const
  {
    IpnElem *operand2 = stack->Pop();
    IpnInt *i2 = dynamic_cast<IpnInt*>(operand2);
/*    if (!i2)
      throw IpnExNotInt(operand2);*/
    IpnElem *operand1 = stack->Pop();
    IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
/*    if (!i1)
      throw IpnExNotInt(operand1);*/
    int res = BinaryOp(i1->Get(), i2->Get());
    delete operand1;
    delete operand2;
    return new IpnInt(res);
  }
};

class IpnFunPlus: public IpnFun2 {
public:
  IpnFunPlus() {}
  virtual ~IpnFunPlus() {}
  virtual int BinaryOp(int i1, int i2) const {
    return i1 + i2;
  }
};

class IpnFunMinus: public IpnFun2 {
public:
  IpnFunMinus() {}
  virtual ~IpnFunMinus() {}
  virtual int BinaryOp(int i1, int i2) const {
    return i1 - i2;
  }
};

class IpnFunMultiplication: public IpnFun2 {
public:
  IpnFunMultiplication() {}
  virtual ~IpnFunMultiplication() {}
  virtual int BinaryOp(int i1, int i2) const {
    return i1 * i2;
  }
};

class IpnFunDivision: public IpnFun2 {
public:
  IpnFunDivision() {}
  virtual ~IpnFunDivision() {}
  virtual int BinaryOp(int i1, int i2) const {
    return (int)(i1 / i2);
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
};

class IpnTakeValue: public IpnElem {
public:
  IpnTakeValue() {}
  virtual ~IpnTakeValue() {}
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd) const
  {
    IpnElem *operand = stack->Pop();
    IpnVarAddr *variable = dynamic_cast<IpnVarAddr*>(operand);
    int *value;
    /*if (!variable)
      throw IpnExcNotVar(operand);*/
    value = variable->Get();
    /*
    if (!value)
      throw IpnExcVarNotDeclared(operand);
    */
    stack->Push(new IpnInt(*value));
    delete operand;
    *cur_cmd = (*cur_cmd)->next;
  }
};

class IpnOpGo: public IpnElem {
public:
  IpnOpGo() {}
  virtual ~IpnOpGo() {}
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd) const
  {
    IpnElem *operand1 = stack->Pop();
    IpnLabel *lab = dynamic_cast<IpnLabel*>(operand1);
  /*  if (!lab)
      throw IpnExNotLabel(operand1);*/
    IpnItem *addr = lab->Get();
    *cur_cmd = addr;
    delete operand1;
  }
};

class IpnOpGoFalse: public IpnElem {
public:
  IpnOpGoFalse() {}
  virtual ~IpnOpGoFalse() {}
  virtual void Evaluate(IpnElemStack *stack, IpnItem **cur_cmd) const
  {
    IpnElem *operand1 = stack->Pop();
    IpnLabel *lab = dynamic_cast<IpnLabel*>(operand1);
  /*  if (!lab)
      throw IpnExNotLabel(operand1);*/
    IpnElem *operand2 = stack->Pop();
    IpnInt *clause = dynamic_cast<IpnInt*>(operand2);
  /*  if (!clause)
      throw IpnExNotInt(operand2);*/
    if (!clause->Get()) {
      IpnItem *addr = lab->Get();
      *cur_cmd = addr;
    } else {
      *cur_cmd = (*cur_cmd)->next;
    }
    delete operand2;
    delete operand1;
  }
};

class SyntException {
private:
  enum { str_size = 1024 };
  char str[str_size];
public:
  SyntException(const char *err_str, const char *lex, int line) {
    strcpy(str, err_str);
    sprintf(str + strlen(str), " in line %d: '%s'", line, lex);
  }
  SyntException(const char *str1) { strcpy(str, str1); }
  const char *ErrorString() { return str; }
};

#endif
