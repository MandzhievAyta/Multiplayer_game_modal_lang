#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum { divider, identificator, key_word, number, const_string };

class Lexeme {
private:
  char *lex;
  int lex_type;
  int line_num;
public:
  Lexeme(): lex(NULL) {}
  Lexeme(const Lexeme &);
  Lexeme(const char *l, int type, int line);
  ~Lexeme();
  void operator=(const Lexeme &);
  const char *GetString() const { return lex; }
  int GetType() const { return lex_type; }
  int GetLineNum() const { return line_num; }
};

Lexeme::Lexeme(const Lexeme &src)
{
  lex = new char[strlen(src.lex)+1];
  strcpy(lex, src.lex);
  lex_type = src.lex_type;
  line_num = src.line_num;
}

Lexeme::Lexeme(const char *l, int type, int line)
{
  lex = new char[strlen(l)+1];
  strcpy(lex, l);
  lex_type = type;
  line_num = line;
}

Lexeme::~Lexeme()
{
  if (lex)
    delete[] lex;
}

void Lexeme::operator=(const Lexeme &src)
{
  if (this != &src) {
    lex = new char[strlen(src.lex)+1];
    strcpy(lex, src.lex);
    lex_type = src.lex_type;
    line_num = src.line_num;
  }
}

class ListOfLexeme {
private:
  struct Node {
    Lexeme value;
    Node *next;
    Node(): next(NULL) {}
  };
  Node *first;
  Node *last;
  Node *cur;
  ListOfLexeme(const ListOfLexeme &);
  void operator=(const ListOfLexeme &);
public:
  ListOfLexeme(): first(NULL), last(NULL) {}
  ~ListOfLexeme();
  void Append(const Lexeme &object);
  void StartIter() { cur = first; }
  Lexeme *GetNext();
};

ListOfLexeme::~ListOfLexeme()
{
  Node *tmp;
  while (first) {
    tmp = first->next;
    delete first;
    first = tmp;
  }
  last = NULL;
}

void ListOfLexeme::Append(const Lexeme &object)
{
  if (!first) {
    first = new Node;
    last = first;
  } else {
    last->next = new Node;
    last = last->next;
  }
  last->value = object;
}

Lexeme *ListOfLexeme::GetNext()
{
  Node *tmp;
  if (!cur)
    return NULL;
  tmp = cur;
  cur = cur->next;
  return &(tmp->value);
}

class Automatic {
private:
  enum { buf_size = 4096 };
  enum { beg_state, number_state, ident_state, keywrd_state, str_state,
         div_state, err_state };
  enum { have_prev_sym, no_prev_sym };
  int state;
  char buf[buf_size];
  int buf_cur_size;
  char prev_sym;
  int prev_sym_state;
  int line_number;
  static const char *words[];
  static const char *dividers[];
  bool IsDivider(const char *);
  bool IsKeyWord(const char *);
  bool CheckSpaceSymbol(char);
  bool CheckDividerSymbol(char);
  void SwitchToError(char);
  Lexeme *HandleDividers(char, int);
  Lexeme *Begin(char);
  Lexeme *Number(char);
  Lexeme *Identificator(char);
  Lexeme *KeyWord(char);
  Lexeme *String(char);
  Lexeme *Divider(char);
  Lexeme *Error(char);
  Lexeme *HandleSym(char);
public:
  Automatic();
  Lexeme *FeedChar(char);
  const char *IsError();
};

const char *Automatic::words[] = { "if", "else", "print", "buy","sell",
                                   "prod", "build", "endturn", "while", NULL };
const char *Automatic::dividers[] = { "+", "-", "*", "/", "<", ">",
                                      "==", ">=", "<=", "<>", "&", "|", "!",
                                      "(", ")", "[", "]", "{", "}", ";",
                                      ":=", ",", NULL };

bool Automatic::IsDivider(const char *str) {
  bool check = false;
  int i;
  for(i = 0; (dividers[i] != NULL) && (!check); i++) {
    if (!strcmp(str, dividers[i]))
      check = true;
  }
  return check;
}

bool Automatic::IsKeyWord(const char *str)
{
  bool check = false;
  int i;
  for(i = 0; (words[i] != NULL) && (!check); i++) {
    if (!strcmp(str, words[i]))
      check = true;
  }
  return check;
}

bool Automatic::CheckSpaceSymbol(char sym)
{
  if ((sym ==  '\n') || (sym ==  ' ') || (sym ==  '\t') || (sym ==  '\r'))
    return true;
  else
    return false;
}

bool Automatic::CheckDividerSymbol(char sym)
{
  if ((sym ==  '+') || (sym ==  '-') || (sym ==  '*') || (sym ==  '/') ||
      (sym ==  '<') || (sym ==  '>') || (sym ==  '=') ||
      (sym ==  '&') || (sym ==  '|') || (sym ==  '!') || (sym ==  '(') ||
      (sym ==  ')') || (sym ==  '[') || (sym ==  ']') || (sym ==  '{') ||
      (sym ==  '}') || (sym ==  ';') || (sym ==  ':') || (sym ==  ','))
    return true;
  else
    return false;
}

void Automatic::SwitchToError(char sym)
{
  char *str = new char[buf_cur_size + 2];
  buf[buf_cur_size] = sym;
  buf[buf_cur_size+1] = '\0';
  buf_cur_size += 2;
  strcpy(str, buf);
  sprintf(buf, "Error occurred in line %d: '%s'", line_number, str);
  state = err_state;
}

const char *Automatic::IsError()
{
  if (state == beg_state)
    return NULL;
  if (state == str_state)
    sprintf(buf, "Unmatched quotes in the end");
  return buf;
}

Lexeme *Automatic::HandleDividers(char sym, int type)
{
  Lexeme *p;
  prev_sym = sym;
  prev_sym_state = have_prev_sym;
  buf[buf_cur_size] = '\0';
  p = new Lexeme(buf, type, line_number);
  state = beg_state;
  return p;
}

Lexeme *Automatic::Begin(char sym)
{
  if ((sym <= '9') && (sym >= '0')) {
    state = number_state;
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  } else
  if ((sym == '?') || (sym == '$')) {
    state = ident_state;
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  } else
  if ((sym >= 'a') && (sym <= 'z')) {
    state = keywrd_state;
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  } else
  if (sym == '"') {
    state = str_state;
  } else
  if (CheckDividerSymbol(sym)) {
    state = div_state;
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  } else
  if (CheckSpaceSymbol(sym)) {
  } else
    SwitchToError(sym);
  return NULL;
}

Lexeme *Automatic::Number(char sym)
{
  if (sym >= '0' && sym <= '9') {
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  } else
  if (CheckDividerSymbol(sym) || CheckSpaceSymbol(sym)) {
    return HandleDividers(sym, number);
  } else {
    SwitchToError(sym);
  }
  return NULL;
}
Lexeme *Automatic::Identificator(char sym)
{
  if ((sym >= '0' && sym <= '9') || (sym >= 'a' && sym <= 'z') ||
      (sym >= 'A' && sym <= 'Z') || (sym == '_')) {
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  } else
  if (CheckDividerSymbol(sym) || CheckSpaceSymbol(sym)) {
    return HandleDividers(sym, identificator);
  } else {
    SwitchToError(sym);
  }
  return NULL;
}

Lexeme *Automatic::KeyWord(char sym)
{
  if ((sym >= 'a' && sym <= 'z') || (sym >= 'A' && sym <= 'Z')) {
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  } else
  if (CheckDividerSymbol(sym) || CheckSpaceSymbol(sym)) {
    buf[buf_cur_size] = '\0';
    if (IsKeyWord(buf))
      return HandleDividers(sym, key_word);
    else
      SwitchToError(sym);
  } else {
    SwitchToError(sym);
  }
  return NULL;
}

Lexeme *Automatic::String(char sym)
{
  Lexeme *p = NULL;
  if (sym == '"') {
    buf[buf_cur_size] = '\0';
    p = new Lexeme(buf, const_string, line_number);
    state = beg_state;
    return p;
  } else {
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  }
  return NULL;
}

Lexeme *Automatic::Divider(char sym)
{
  Lexeme *p = NULL;
  buf[buf_cur_size] = sym;
  buf[buf_cur_size+1] = '\0';
  if (IsDivider(buf)) {
    p = new Lexeme(buf, divider, line_number);
    state = beg_state;
  } else {
    buf[buf_cur_size] = '\0';
    if (IsDivider(buf)) {
      p = new Lexeme(buf, divider, line_number);
      prev_sym = sym;
      prev_sym_state = have_prev_sym;
      state = beg_state;
    } else
      SwitchToError(sym);
  }
  return p;
}

Lexeme *Automatic::Error(char sym)
{
  return NULL;
}

Lexeme *Automatic::HandleSym(char sym)
{
  Lexeme *p = NULL;
  switch (state) {
    case beg_state:
      p = Begin(sym);
      break;
    case number_state:
      p = Number(sym);
      break;
    case ident_state:
      p = Identificator(sym);
      break;
    case keywrd_state:
      p = KeyWord(sym);
      break;
    case str_state:
      p = String(sym);
      break;
    case div_state:
      p = Divider(sym);
      break;
    case err_state:
      p = Error(sym);
      break;
  }
  return p;
}

Automatic::Automatic()
{
  state = beg_state;
  prev_sym_state = no_prev_sym;
  buf_cur_size = 0;
  line_number = 1;
}

Lexeme *Automatic::FeedChar(char sym)
{
  Lexeme *p = NULL;
  if (prev_sym_state == have_prev_sym) {
    p = HandleSym(prev_sym);
    prev_sym_state = no_prev_sym;
  }
  p = HandleSym(sym);
  if (p) buf_cur_size = 0;
  if (sym == '\n')
    line_number++;
  return p;
}

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

ListOfIpnItem::~ListOfIpnItem()
{
  IpnItem *tmp;
  while (first) {
    tmp = first->next;
    if (first->p)
      delete first->p;
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
/*
IpnElem *ListOfIpnItem::GetNext()
{
  IpnItem *tmp;
  if (!cur)
    return NULL;
  tmp = cur;
  cur = cur->next;
  return tmp->p;
}
*/
class IpnElemStack {
private:
  IpnItem *vertex;
public:
  IpnElemStack(): vertex(NULL) {}
  ~IpnElemStack();
  void Push(IpnElem *elem);
  IpnElem *Pop();
};

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

IpnElem *IpnElemStack::Pop() {
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

class SyntaxAnalyzer {
private:
  Lexeme *cur_lex;
  ListOfLexeme &list;
  IpnElemStack *stack;
  ListOfIpnItem *ipn_list;
  ListOfVar *list_var;
  void Next() { cur_lex = list.GetNext(); }
  void NextNotNull();
  void CheckLexeme(const char *, const char *);
  void Program();
  void Block();
  void HandleKeyWords();
  void Line();
  IpnElem *DefineIpnFun();
  IpnElem *DefineIpnElem();
  void Expr();
  void Ari1();
  void Ari1Alternative();
  void Ari2();
  void Ari3();
  void PrintArg();
  void Variable();
public:
  SyntaxAnalyzer(ListOfLexeme &l1);
  ~SyntaxAnalyzer() { delete stack; }
  ListOfIpnItem *Check(ListOfVar *a);
};

SyntaxAnalyzer::SyntaxAnalyzer(ListOfLexeme &l1): list(l1)
{
  stack = new IpnElemStack;
  ipn_list = new ListOfIpnItem;
  list.StartIter();
}
ListOfIpnItem *SyntaxAnalyzer::Check(ListOfVar *a)
{
  list_var = a;
  Program();
  return ipn_list;
}

void SyntaxAnalyzer::NextNotNull()
{
  Next();
  if (!cur_lex)
    throw SyntException("Unexpected end of program in last line");
}
void SyntaxAnalyzer::CheckLexeme(const char *lex, const char *excep_info)
{
  if (strcmp(cur_lex->GetString(), lex))
    throw SyntException(excep_info, cur_lex->GetString(),
                    cur_lex->GetLineNum());
  else
    NextNotNull();
}

void SyntaxAnalyzer::Program()
{
#if defined(PRINT_MODE)
  printf("<Program>->");
#endif
  try {
    NextNotNull();
    Line();
    while (!strcmp(cur_lex->GetString(), ";")) {
      Next();
      if (!cur_lex)
        break;
      Line();
    }
    if (cur_lex != NULL)
      throw SyntException("Missed ';'", cur_lex->GetString(),
                      cur_lex->GetLineNum());
    printf("Syntax analyzer correct");
  }
  catch (SyntException &e) {
    printf(e.ErrorString());
  }
  putchar('\n');
}

void SyntaxAnalyzer::Block()
{
#if defined(PRINT_MODE)
  printf("<Block>->");
#endif
  CheckLexeme("{", "Missed '{' in block");
  Line();
  while (!strcmp(cur_lex->GetString(), ";")) {
    NextNotNull();
    if (!strcmp("}", cur_lex->GetString()))
      break;
    Line();
  }
  CheckLexeme("}", "Missed } in block");
}

void SyntaxAnalyzer::HandleKeyWords()
{
  if (!strcmp("print", cur_lex->GetString()))
  {
    NextNotNull();
    CheckLexeme("(", "Missed '(' after 'print'");
    ipn_list->Append(new IpnOpenBracket);
#if defined(PRINT_MODE_POLIZ)
    printf(" ( ");
#endif
    PrintArg();
    while (!strcmp(cur_lex->GetString(), ",")) {
      NextNotNull();
      PrintArg();
    }
    CheckLexeme(")", "Missed ')' after 'print'");
    ipn_list->Append(new IpnPrint);
#if defined(PRINT_MODE_POLIZ)
    printf(" Print ");
#endif
  } else
  if (!strcmp("buy", cur_lex->GetString()) ||
      !strcmp("sell", cur_lex->GetString()))
  {
    NextNotNull();
    Expr();
    Expr();
  } else
  if (!strcmp("build", cur_lex->GetString()) ||
      !strcmp("prod", cur_lex->GetString()))
  {
    NextNotNull();
    Expr();
  } else
  if (!strcmp("endturn", cur_lex->GetString()))
    NextNotNull();
}

void SyntaxAnalyzer::Line()
{
  IpnItem *l1, *l2;
#if defined(PRINT_MODE)
  printf("<Line>->");
#endif
  if ((cur_lex->GetType() == identificator) &&
      (cur_lex->GetString()[0] == '$')) {
    Variable();
    CheckLexeme(":=", "Should be assignment");
    Expr();
    ipn_list->Append(new IpnFunAssign);
#if defined(PRINT_MODE_POLIZ)
    printf(" := ");
#endif
  } else
  if (!strcmp("if", cur_lex->GetString())) {
    NextNotNull();
    Expr();
    ipn_list->Append(NULL);
    l1 = ipn_list->GetAddr();
#if defined(PRINT_MODE_POLIZ)
    printf(" l1 ");
#endif
    ipn_list->Append(new IpnOpGoFalse);
#if defined(PRINT_MODE_POLIZ)
    printf(" !F ");
#endif
    Line();
    ipn_list->Append(NULL);
    l2 = ipn_list->GetAddr();
#if defined(PRINT_MODE_POLIZ)
    printf(" l2 ");
#endif
    ipn_list->Append(new IpnOpGo);
#if defined(PRINT_MODE_POLIZ)
    printf(" ! ");
#endif
    if (!strcmp("else", cur_lex->GetString())) {
      ipn_list->Append(new IpnNoOp);
      l1->p = new IpnLabel(ipn_list->GetAddr());
#if defined(PRINT_MODE_POLIZ)
      printf(" l1: ");
#endif
      NextNotNull();
      Line();
    }
    ipn_list->Append(new IpnNoOp);
    l2->p = new IpnLabel(ipn_list->GetAddr());
#if defined(PRINT_MODE_POLIZ)
    printf(" l2: ");
#endif
    if (!l1->p) {
      l1->p = l2->p;
#if defined(PRINT_MODE_POLIZ)
      printf(" l1: ");
#endif
    }
  } else
  if (!strcmp("while", cur_lex->GetString())) {
    NextNotNull();
    ipn_list->Append(new IpnNoOp);
    l2 = ipn_list->GetAddr();
#if defined(PRINT_MODE_POLIZ)
    printf(" l2: ");
#endif
    CheckLexeme("(", "Missed '(' after 'while'");
    Expr();
    CheckLexeme(")", "Missed ')' after 'while'");
    ipn_list->Append(NULL);
    l1 = ipn_list->GetAddr();
#if defined(PRINT_MODE_POLIZ)
    printf(" l1 ");
#endif
    ipn_list->Append(new IpnOpGoFalse);
#if defined(PRINT_MODE_POLIZ)
    printf(" !F ");
#endif
    Line();
    ipn_list->Append(new IpnLabel(l2));
#if defined(PRINT_MODE_POLIZ)
    printf(" l2 ");
#endif
    ipn_list->Append(new IpnOpGo());
#if defined(PRINT_MODE_POLIZ)
    printf(" ! ");
#endif
    ipn_list->Append(new IpnNoOp);
    l1->p = new IpnLabel(ipn_list->GetAddr());
#if defined(PRINT_MODE_POLIZ)
    printf(" l1: ");
#endif
  } else
  if (!strcmp("{", cur_lex->GetString())) {
    Block();
  } else
  if (cur_lex->GetType() == key_word) {
    HandleKeyWords();
  } else
    throw SyntException("Line can not start with this lexeme",
                        cur_lex->GetString(), cur_lex->GetLineNum());
}

IpnElem *SyntaxAnalyzer::DefineIpnFun()
{
  const char *str_lex = cur_lex->GetString();
  IpnElem *elem;
  if (!strcmp("?sell", str_lex)) {
    elem = new IpnFunGreater;
  } else
  if (!strcmp("<", str_lex)) {
    elem = new IpnFunLess;
  } else
  if (!strcmp("<>", str_lex)) {
    elem = new IpnFunNotEqual;
  } else
  if (!strcmp(">=", str_lex)) {
    elem = new IpnFunGreaterEqual;
  } else
  if (!strcmp("<=", str_lex)) {
    elem = new IpnFunLessEqual;
  } else
  if (!strcmp("==", str_lex)) {
    elem = new IpnFunEqual;
/*  } else
  if (!strcmp("(", str_lex)) {
    elem = new IpnOpenBracket;} else*/
  } else
  if (!strcmp("+", str_lex)) {
    elem = new IpnFunPlus;
  } else
  if (!strcmp("-", str_lex)) {
    elem = new IpnFunMinus;
  } else
  if (!strcmp("*", str_lex)) {
    elem = new IpnFunMultiplication;
  } else
  if (!strcmp("/", str_lex)) {
    elem = new IpnFunDivision;
  } else
  if (!strcmp("|", str_lex)) {
    elem = new IpnFunLogOr;
  } else
  if (!strcmp("&", str_lex)) {
    elem = new IpnFunLogAnd;
  }
  return elem;
}

IpnElem *SyntaxAnalyzer::DefineIpnElem()
{
  const char *str_lex = cur_lex->GetString();
  IpnElem *elem;
  if (!strcmp(">", str_lex)) {
    elem = new IpnFunGreater;
  } else
  if (!strcmp("<", str_lex)) {
    elem = new IpnFunLess;
  } else
  if (!strcmp("<>", str_lex)) {
    elem = new IpnFunNotEqual;
  } else
  if (!strcmp(">=", str_lex)) {
    elem = new IpnFunGreaterEqual;
  } else
  if (!strcmp("<=", str_lex)) {
    elem = new IpnFunLessEqual;
  } else
  if (!strcmp("==", str_lex)) {
    elem = new IpnFunEqual;
/*  } else
  if (!strcmp("(", str_lex)) {
    elem = new IpnOpenBracket;*/
  } else
  if (!strcmp("+", str_lex)) {
    elem = new IpnFunPlus;
  } else
  if (!strcmp("-", str_lex)) {
    elem = new IpnFunMinus;
  } else
  if (!strcmp("*", str_lex)) {
    elem = new IpnFunMultiplication;
  } else
  if (!strcmp("/", str_lex)) {
    elem = new IpnFunDivision;
  } else
  if (!strcmp("|", str_lex)) {
    elem = new IpnFunLogOr;
  } else
  if (!strcmp("&", str_lex)) {
    elem = new IpnFunLogAnd;
  }
  return elem;
}

void SyntaxAnalyzer::Expr()
{
#if defined(PRINT_MODE)
  printf("<Expr>->");
#endif
  Ari1();
  Ari1Alternative();
}

void SyntaxAnalyzer::Ari1Alternative()
{
  IpnElem *elem;
  const char *str_lex = cur_lex->GetString();
#if defined(PRINT_MODE)
  printf("<Ari1Alternative>->");
#endif
  if (!strcmp(">", str_lex) ||
      !strcmp("<", str_lex) ||
      !strcmp("<>", str_lex) ||
      !strcmp(">=", str_lex) ||
      !strcmp("<=", str_lex) ||
      !strcmp("==", str_lex))
  {
    elem = DefineIpnElem();
    NextNotNull();
    Ari1();
    ipn_list->Append(elem);
#if defined(PRINT_MODE_POLIZ)
    printf(" %s ", str_lex);
#endif
  }
}

void SyntaxAnalyzer::Ari1()
{
  IpnElem *elem;
  const char *str_lex;
#if defined(PRINT_MODE)
  printf("<Ari1>->");
#endif
  Ari2();
  str_lex = cur_lex->GetString();
  while (!strcmp("+", str_lex) ||
         !strcmp("-", str_lex) ||
         !strcmp("|", str_lex))
  {
    elem = DefineIpnElem();
    NextNotNull();
    Ari2();
#if defined(PRINT_MODE_POLIZ)
    printf(" %s ", str_lex);
#endif
    str_lex = cur_lex->GetString();
    ipn_list->Append(elem);
  }
}

void SyntaxAnalyzer::Ari2()
{
  IpnElem *elem;
  const char *str_lex;
#if defined(PRINT_MODE)
  printf("<Ari2>->");
#endif
  Ari3();
  str_lex = cur_lex->GetString();
  while (!strcmp("*", str_lex) ||
         !strcmp("/", str_lex) ||
         !strcmp("&", str_lex))
  {
    elem = DefineIpnElem();
    NextNotNull();
    Ari3();
#if defined(PRINT_MODE_POLIZ)
    printf(" %s ", str_lex);
#endif
    str_lex = cur_lex->GetString();
    ipn_list->Append(elem);
  }
}

void SyntaxAnalyzer::Ari3()
{
#if defined(PRINT_MODE)
  printf("<Ari3>->");
#endif
  if (cur_lex->GetString()[0] == '$') {
    Variable();
    ipn_list->Append(new IpnTakeValue);
#if defined(PRINT_MODE_POLIZ)
    printf(" @ ");
#endif
  } else
  if (cur_lex->GetString()[0] == '?') {
    NextNotNull();
    CheckLexeme("(", "Missed '(' after '?function'");
    if (strcmp(")", cur_lex->GetString())) {
      Expr();
      while (!strcmp(",", cur_lex->GetString())) {
        NextNotNull();
        Expr();
      }
    }
    CheckLexeme(")", "Missed ')' after '?function'");
  } else
  if (cur_lex->GetType() == number) {
    ipn_list->Append(new IpnInt(atoi(cur_lex->GetString())));
#if defined(PRINT_MODE_POLIZ)
    printf(" %s ", cur_lex->GetString());
#endif
    NextNotNull();
  } else
  if (!strcmp("!", cur_lex->GetString())) {
    NextNotNull();
    Ari3();
    ipn_list->Append(new IpnFunLogNot);
#if defined(PRINT_MODE_POLIZ)
    printf(" NOT ");
#endif
  } else
  if (!strcmp("(", cur_lex->GetString())) {
    NextNotNull();
    Expr();
    CheckLexeme(")", "Missed ')' after expression");
  } else
    throw SyntException("Wrong arithmetic member", cur_lex->GetString(),
                    cur_lex->GetLineNum());
}

void SyntaxAnalyzer::PrintArg()
{
#if defined(PRINT_MODE)
  printf("<PrintArg>->");
#endif
  if (cur_lex->GetType() == const_string) {
    ipn_list->Append(new IpnString(cur_lex->GetString()));
#if defined(PRINT_MODE_POLIZ)
    printf(" %s ", cur_lex->GetString());
#endif
    NextNotNull();
  } else
    Expr();
}

void SyntaxAnalyzer::Variable()
{
#if defined(PRINT_MODE)
  printf("<Variable>->");
#endif
  ipn_list->Append(new IpnVarAddr(list_var, cur_lex->GetString()));
#if defined(PRINT_MODE_POLIZ)
  printf(" %s ", cur_lex->GetString());
#endif
  NextNotNull();
  if (!strcmp("[", cur_lex->GetString())) {
    NextNotNull();
    Expr();
    CheckLexeme("]", "Missed ']' after array");
    ipn_list->Append(new IpnFunAddDimension);
#if defined(PRINT_MODE_POLIZ)
    printf(" [] ");
#endif
  }
}

FILE *OpenFile(const char *path)
{
  FILE *file_in;
  file_in = fopen(path, "r");
  if (file_in == NULL) {
    perror(path);
    exit(1);
  }
  return file_in;
}

void PrintListLexeme(ListOfLexeme &list)
{
  Lexeme *lex;
  list.StartIter();
  lex = list.GetNext();
  while (lex) {
    printf("line: %d, type: %d, %s\n", lex->GetLineNum(), lex->GetType(),
           lex->GetString());
    lex = list.GetNext();
  }
}

void CheckError(Automatic &machine)
{
  const char *err;
  err = machine.IsError();
  if (err)
    printf(err);
  else
    printf("Correct!");
  putchar('\n');
}

void HandleNextLexeme(ListOfLexeme &list, Lexeme *lex)
{
  if (lex) {
    list.Append(*lex);
    delete lex;
  }
}

int main(int argc, char **argv)
{
  char current_sym;
  FILE *file_in;

  Automatic machine;
  SyntaxAnalyzer *syntax;
  Lexeme *lex = NULL;
  ListOfLexeme list_lex;

  ListOfVar *list_var = new ListOfVar;
  ListOfIpnItem *ipn_list;
  IpnElemStack intepr_stack;
  IpnItem *cur_cmd;

  file_in = OpenFile(argv[1]);
  while ((current_sym = fgetc(file_in)) != EOF) {
    lex = machine.FeedChar(current_sym);
    HandleNextLexeme(list_lex, lex);
  }
  lex = machine.FeedChar(' ');
  HandleNextLexeme(list_lex, lex);
#if defined(PRINT_MODE)
  PrintListLexeme(list_lex);
#endif
  CheckError(machine);
  syntax = new SyntaxAnalyzer(list_lex);
  ipn_list = syntax->Check(list_var);
  cur_cmd = ipn_list->GetFirst();
  while (cur_cmd) {
    (*cur_cmd).p->Evaluate(&intepr_stack, &cur_cmd);
  }
  return 0;
}
