#include "lexemes.h"
#include "ipnclasses.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ListOfVar::~ListOfVar()
{
  Node *tmp;
  while (first) {
    tmp = first->next;
    delete first;
    first = tmp;
  }
  last = NULL;
}

void ListOfVar::Change(const char *varname, int newvalue)
{
  Node **tmp = &first;
  while (*tmp) {
    if (!strcmp((**tmp).name, varname)) {
      (**tmp).value = newvalue;
      return;
    }
    tmp = &((**tmp).next);
  }
  *tmp = new Node;
  last = *tmp;
  last->name = new char[strlen(varname)+1];
  strcpy(last->name, varname);
  last->value = newvalue;
}


int *ListOfVar::Get(const char *varname)
{
  Node **tmp = &first;
  while (*tmp) {
    if (!strcmp((**tmp).name, varname)) {
      return &(**tmp).value;
    }
    tmp = &((**tmp).next);
  }
  return NULL;
}

ListOfIpnItem::~ListOfIpnItem()
{
  IpnItem *tmp;
  while (first) {
    tmp = first->next;
    if (first->p)
      delete (first->p);
    delete first;
    first = tmp;
  }
  last = NULL;
}

void ListOfIpnItem::Append(IpnElem *object)
{
  if (!first) {
    first = new IpnItem;
    last = first;
  } else {
    last->next = new IpnItem;
    last = last->next;
  }
  last->p = object;
}

IpnElemStack::~IpnElemStack()
{
  IpnItem *tmp;
  while (vertex) {
    tmp = vertex->next;
    if (vertex->p)
      delete vertex->p;
    delete vertex;
    vertex = tmp;
  }
}

void IpnElemStack::Push(IpnElem *elem)
{
  IpnItem *tmp;
  if (!vertex) {
    vertex = new IpnItem;
  } else {
    tmp = new IpnItem;
    tmp->next = vertex;
    vertex = tmp;
  }
  vertex->p = elem;
}

IpnElem *IpnElemStack::Pop()
{
  IpnElem *tmp;
  IpnItem *tmp_item;
  if (!vertex)
    return NULL;
  tmp = vertex->p;
  tmp_item = vertex;
  vertex = vertex->next;
  delete tmp_item;
  return tmp;
}

void IpnVarAddr::AddDimension(int dim)
{
  char string[1024], *tmp;
  sprintf(string, "%d", dim);
  tmp = new char[strlen(name) + strlen(string) + 3];
  sprintf(tmp, "%s[%d]", name, dim);
  delete[] name;
  name = tmp;
}

void IpnPrint::Evaluate(IpnElemStack *stack, IpnItem **cur_cmd,
                        GameContext &context) const
{
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

IpnElem *IpnSell::EvaluateFun(IpnElemStack *stack,
                              GameContext &context) const
{
  char string[1024];
  IpnElem *operand2 = stack->Pop();
  IpnInt *i2 = dynamic_cast<IpnInt*>(operand2);
  if (!i2)
    throw IpnExNotInt(operand2);
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  if (!i1)
    throw IpnExNotInt(operand1);
  sprintf(string, "sell %d %d\n", i1->Get(), i2->Get());
  write(context.sockfd, string, strlen(string));
  delete operand1;
  delete operand2;
  return NULL;
}

IpnElem *IpnBuy::EvaluateFun(IpnElemStack *stack,
                             GameContext &context) const
{
  char string[1024];
  IpnElem *operand2 = stack->Pop();
  IpnInt *i2 = dynamic_cast<IpnInt*>(operand2);
  if (!i2)
    throw IpnExNotInt(operand2);
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  if (!i1)
    throw IpnExNotInt(operand1);
  sprintf(string, "buy %d %d\n", i1->Get(), i2->Get());
  write(context.sockfd, string, strlen(string));
  delete operand1;
  delete operand2;
  return NULL;
}

IpnElem *IpnProd::EvaluateFun(IpnElemStack *stack,
                              GameContext &context) const
{
  char string[1024];
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  if (!i1)
    throw IpnExNotInt(operand1);
  sprintf(string, "prod %d\n", i1->Get());
  write(context.sockfd, string, strlen(string));
  delete operand1;
  return NULL;
}

IpnElem *IpnBuild::EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const
{
  char string[1024];
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  if (!i1)
    throw IpnExNotInt(operand1);
  sprintf(string, "build %d\n", i1->Get());
  write(context.sockfd, string, strlen(string));
  delete operand1;
  return NULL;
}

IpnElem *IpnMoney::EvaluateFun(IpnElemStack *stack,
                               GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  int indx;
  if (!i1)
    throw IpnExNotInt(operand1);
  indx = i1->Get();
  delete operand1;
  if (indx > context.market.max_cl)
    return new IpnInt(-1);
  if (context.market.players[indx - 1].state == 0)
    return new IpnInt(-1);
  else
    return new IpnInt(context.market.players[indx - 1].mon);
}

IpnElem *IpnRaw::EvaluateFun(IpnElemStack *stack,
                             GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  int indx;
  if (!i1)
    throw IpnExNotInt(operand1);
  indx = i1->Get();
  delete operand1;
  if (indx > context.market.max_cl)
    return new IpnInt(-1);
  if (context.market.players[indx - 1].state == 0)
    return new IpnInt(-1);
  else
    return new IpnInt(context.market.players[indx - 1].row);
}

IpnElem *IpnProduction::EvaluateFun(IpnElemStack *stack,
                                    GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  int indx;
  if (!i1)
    throw IpnExNotInt(operand1);
  indx = i1->Get();
  delete operand1;
  if (indx > context.market.max_cl)
    return new IpnInt(-1);
  if (context.market.players[indx - 1].state == 0)
    return new IpnInt(-1);
  else
    return new IpnInt(context.market.players[indx - 1].prod);
}

IpnElem *IpnFactories::EvaluateFun(IpnElemStack *stack,
                                   GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  int indx;
  if (!i1)
    throw IpnExNotInt(operand1);
  indx = i1->Get();
  delete operand1;
  if (indx > context.market.max_cl)
    return new IpnInt(-1);
  if (context.market.players[indx - 1].state == 0)
    return new IpnInt(-1);
  else
    return new IpnInt(context.market.players[indx - 1].fact);
}

IpnElem *IpnManufactured::EvaluateFun(IpnElemStack *stack,
                                      GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  int indx;
  if (!i1)
    throw IpnExNotInt(operand1);
  indx = i1->Get();
  delete operand1;
  if (indx > context.market.max_cl)
    return new IpnInt(-1);
  if (context.market.players[indx - 1].state == 0)
    return new IpnInt(-1);
  else
    return new IpnInt(context.market.players[indx - 1].b_fact);
}

IpnElem *IpnResultRawSold::EvaluateFun(IpnElemStack *stack,
                                       GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  int indx, res = 0, j;
  InfoBet *p;
  if (!i1)
    throw IpnExNotInt(operand1);
  indx = i1->Get();
  delete operand1;
  if (indx > context.market.max_cl)
    return new IpnInt(-1);
  p = context.market.sold;
  for (j = 0; j < context.market.max_cl; j++) {
    if (p[j].state == 0)
      break;
    if (p[j].num == indx) {
      res = p[j].amnt;
      break;
    }
  }
  if (context.market.players[indx - 1].state == 0)
    return new IpnInt(-1);
  else
    return new IpnInt(res);
}
IpnElem *IpnResultRawPrice::EvaluateFun(IpnElemStack *stack,
                                        GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  int indx, res = 0, j;
  InfoBet *p;
  if (!i1)
    throw IpnExNotInt(operand1);
  indx = i1->Get();
  delete operand1;
  if (indx > context.market.max_cl)
    return new IpnInt(-1);
  p = context.market.sold;
  for (j = 0; j < context.market.max_cl; j++) {
    if (p[j].state == 0)
      break;
    if (p[j].num == indx) {
      res = p[j].price;
      break;
    }
  }
  if (context.market.players[indx - 1].state == 0)
    return new IpnInt(-1);
  else
    return new IpnInt(res);
}

IpnElem *IpnResultProdBought::EvaluateFun(IpnElemStack *stack,
                                          GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  int indx, res = 0, j;
  InfoBet *p;
  if (!i1)
    throw IpnExNotInt(operand1);
  indx = i1->Get();
  delete operand1;
  if (indx > context.market.max_cl)
    return new IpnInt(-1);
  p = context.market.bought;
  for (j = 0; j < context.market.max_cl; j++) {
    if (p[j].state == 0)
      break;
    if (p[j].num == indx) {
      res = p[j].amnt;
      break;
    }
  }
  if (context.market.players[indx - 1].state == 0)
    return new IpnInt(-1);
  else
    return new IpnInt(res);
}

IpnElem *IpnResultProdPrice::EvaluateFun(IpnElemStack *stack,
                                         GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  int indx, res = 0, j;
  InfoBet *p;
  if (!i1)
    throw IpnExNotInt(operand1);
  indx = i1->Get();
  delete operand1;
  if (indx > context.market.max_cl)
    return new IpnInt(-1);
  p = context.market.bought;
  for (j = 0; j < context.market.max_cl; j++) {
    if (p[j].state == 0)
      break;
    if (p[j].num == indx) {
      res = p[j].price;
      break;
    }
  }
  if (context.market.players[indx - 1].state == 0)
    return new IpnInt(-1);
  else
    return new IpnInt(res);
}

IpnElem *IpnFunAssign::EvaluateFun(IpnElemStack *stack,
                                   GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  if (!i1)
    throw IpnExNotInt(operand1);
  IpnElem *operand2 = stack->Pop();
  IpnVarAddr *i2 = dynamic_cast<IpnVarAddr*>(operand2);
  if (!i2)
    throw IpnExNotVar(operand2);
  i2->Change(i1->Get());
  delete operand1;
  delete operand2;
  return NULL;
}

IpnElem *IpnFunLogNot::EvaluateFun(IpnElemStack *stack,
                                   GameContext &context) const
{
  IpnElem *operand = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand);
  if (!i1)
    throw IpnExNotInt(operand);
  int res = !(i1->Get());
  return new IpnInt(res);
}

IpnElem *IpnFunAddDimension::EvaluateFun(IpnElemStack *stack,
                                         GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  if (!i1)
    throw IpnExNotInt(operand1);
  IpnElem *operand2 = stack->Pop();
  IpnVarAddr *i2 = dynamic_cast<IpnVarAddr*>(operand2);
  if (!i2)
    throw IpnExNotVar(operand1);
  i2->AddDimension(i1->Get());
  delete operand1;
  return i2;
}

IpnElem *IpnFun2::EvaluateFun(IpnElemStack *stack,
                              GameContext &context) const
{
  IpnElem *operand2 = stack->Pop();
  IpnInt *i2 = dynamic_cast<IpnInt*>(operand2);
  if (!i2)
    throw IpnExNotInt(operand2);
  IpnElem *operand1 = stack->Pop();
  IpnInt *i1 = dynamic_cast<IpnInt*>(operand1);
  if (!i1)
    throw IpnExNotInt(operand1);
  int res = BinaryOp(i1->Get(), i2->Get());
  delete operand1;
  delete operand2;
  return new IpnInt(res);
}

void IpnTakeValue::Evaluate(IpnElemStack *stack, IpnItem **cur_cmd,
                            GameContext &context) const
{
  IpnElem *operand = stack->Pop();
  IpnVarAddr *variable = dynamic_cast<IpnVarAddr*>(operand);
  int *value;
  if (!variable)
    throw IpnExNotVar(operand);
  value = variable->Get();
  if (!value)
    throw IpnExVarNotDeclared(operand);
  if (!value)
    stack->Push(new IpnInt(0));
  else
    stack->Push(new IpnInt(*value));
  delete operand;
  *cur_cmd = (*cur_cmd)->next;
}

void IpnOpGo::Evaluate(IpnElemStack *stack, IpnItem **cur_cmd,
                       GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnLabel *lab = dynamic_cast<IpnLabel*>(operand1);
  if (!lab)
    throw IpnExNotLabel(operand1);
  IpnItem *addr = lab->Get();
  *cur_cmd = addr;
  delete operand1;
}

void IpnOpGoFalse::Evaluate(IpnElemStack *stack, IpnItem **cur_cmd,
                            GameContext &context) const
{
  IpnElem *operand1 = stack->Pop();
  IpnLabel *lab = dynamic_cast<IpnLabel*>(operand1);
  if (!lab)
    throw IpnExNotLabel(operand1);
  IpnElem *operand2 = stack->Pop();
  IpnInt *clause = dynamic_cast<IpnInt*>(operand2);
  if (!clause)
    throw IpnExNotInt(operand2);
  if (!clause->Get()) {
    IpnItem *addr = lab->Get();
    *cur_cmd = addr;
  } else {
    *cur_cmd = (*cur_cmd)->next;
  }
  delete operand2;
  delete operand1;
}
