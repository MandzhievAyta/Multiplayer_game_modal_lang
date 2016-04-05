#include <cstdlib>
#include <cstdio>
#include <cstring>

enum { divider, identificator, key_word, number, const_string };

template <class T>
class List {
private:
  struct Node {
    T value;
    Node *next;
    Node(): next(NULL) {}
  };
  Node *first;
  Node *last;
  Node *cur;
  List(const List<T> &);
  void operator=(const List<T> &);
public:
  List(): first(NULL), last(NULL) {}
  ~List();
  void Append(const T &object);
  void StartIter() { cur = first; }
  T *GetNext();
};

template <class T>
List<T>::~List()
{
  Node *tmp;
  while (first) {
    tmp = first->next;
    delete first;
    first = tmp;
  }
  last = NULL;
}

template <class T>
void List<T>::Append(const T &object)
{
  if (first == NULL) {
    first = new Node;
    last = first;
  } else {
    last->next = new Node;
    last = last->next;
  }
  last->value = object;
}

template <class T>
T *List<T>::GetNext()
{
  List<T>::Node *tmp;
  if (!cur)
    return NULL;
  tmp = cur;
  cur = cur->next;
  return &(tmp->value);
}

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

const char *Automatic::words[] = { "if", "then", "else", "print", "buy","sell",
                                   "prod", "build", "endturn", "while", NULL };
const char *Automatic::dividers[] = { "+", "-", "*", "/", "<", ">",
                                      "=", ">=", "<=", "<>", "&", "|", "!",
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
      (sym ==  '%') || (sym ==  '<') || (sym ==  '>') || (sym ==  '=') ||
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
  } else if ((sym == '?') || (sym == '$')) {
    state = ident_state;
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  } else if ((sym >= 'a') && (sym <= 'z')) {
    state = keywrd_state;
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  } else if (sym == '"') {
    state = str_state;
  } else if (CheckDividerSymbol(sym)) {
    state = div_state;
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  } else if (CheckSpaceSymbol(sym)) {
  } else
    SwitchToError(sym);
  return NULL;
}

Lexeme *Automatic::Number(char sym)
{
  if (sym >= '0' && sym <= '9') {
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  } else if (CheckDividerSymbol(sym) || CheckSpaceSymbol(sym)) {
    return HandleDividers(sym, number);
  } else {
    SwitchToError(sym);
  }
  return NULL;
}
Lexeme *Automatic::Identificator(char sym)
{
  if ((sym >= '0' && sym <= '9') || (sym >= 'a' && sym <= 'z') ||
      (sym >= 'A' && sym <= 'Z')) {
    buf[buf_cur_size] = sym;
    buf_cur_size++;
  } else if (CheckDividerSymbol(sym) || CheckSpaceSymbol(sym)) {
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
  } else if (CheckDividerSymbol(sym) || CheckSpaceSymbol(sym)) {
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
  char tmp;
  bool this_step = false;
  Lexeme *p = NULL;
  if (prev_sym_state == have_prev_sym) {
    tmp = prev_sym;
    this_step = true;
  } else
    tmp = sym;
  do {
    p = HandleSym(tmp);
    if (tmp == sym) {
      this_step = false;
    } else {
      tmp = sym;
      prev_sym_state = no_prev_sym;
    }
  } while(this_step);
  if (p) buf_cur_size = 0;
  if (sym == '\n')
    line_number++;
  return p;
}

class Exception {
private:
  enum { str_size = 1024 };
  char str[str_size];
public:
  Exception(const char *err_str, const char *lex, int line) {
    strcpy(str, err_str);
    sprintf(str + strlen(str), " in line %d: '%s'", line, lex);
  }
  Exception(const char *str1) { strcpy(str, str1); }
  const char *ErrorString() { return str; }
};

class SyntaxAnalyzer {
private:
  Lexeme *cur_lex;
  List<Lexeme> &list;
  void Next() { cur_lex = list.GetNext(); }
  void NextNotNull();
  void CheckLexeme(const char *, const char *);
  void Program();
  void Block();
  void HandleKeyWords();
  void Line();
  void Expr();
  void Ari1();
  void Ari2();
  void Ari3();
  void PrintArg();
  void Variable();
public:
  SyntaxAnalyzer(List<Lexeme> &l1): list(l1) { list.StartIter(); }
  void Check() { Program(); }
};

void SyntaxAnalyzer::NextNotNull()
{
  Next();
  if (!cur_lex)
    throw Exception("Unexpected end of program in last line");
}
void SyntaxAnalyzer::CheckLexeme(const char *lex, const char *excep_info)
{
  if (strcmp(cur_lex->GetString(), lex))
    throw Exception(excep_info, cur_lex->GetString(),
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
      throw Exception("Missed ';'", cur_lex->GetString(),
                      cur_lex->GetLineNum());
    printf("Syntax analyzer correct");
  }
  catch (Exception &e) {
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
  if (!strcmp("print", cur_lex->GetString())) {
    NextNotNull();
    CheckLexeme("(", "Missed '(' after 'print'");
    PrintArg();
    while (!strcmp(cur_lex->GetString(), ",")) {
      NextNotNull();
      PrintArg();
    }
    CheckLexeme(")", "Missed ')' after 'print'");
  } else if (!strcmp("buy", cur_lex->GetString()) ||
             !strcmp("sell", cur_lex->GetString())) {
    NextNotNull();
    Expr();
    Expr();
  } else if (!strcmp("build", cur_lex->GetString()) ||
             !strcmp("prod", cur_lex->GetString())) {
    NextNotNull();
    Expr();
  } else if (!strcmp("endturn", cur_lex->GetString()))
    NextNotNull();
}

void SyntaxAnalyzer::Line()
{
#if defined(PRINT_MODE)
  printf("<Line>->");
#endif
  if ((cur_lex->GetType() == identificator) &&
      (cur_lex->GetString()[0] == '$')) {
    Variable();
    CheckLexeme(":=", "Should be assignment");
    Expr();
  } else if (!strcmp("if", cur_lex->GetString())) {
    NextNotNull();
    Expr();
    Line();
    if (!strcmp("else", cur_lex->GetString())) {
      NextNotNull();
      Line();
    }
  } else if (!strcmp("while", cur_lex->GetString())) {
    NextNotNull();
    CheckLexeme("(", "Missed '(' after 'while'");
    Expr();
    CheckLexeme(")", "Missed ')' after 'while'");
    Line();
  } else if (!strcmp("{", cur_lex->GetString())) {
    Block();
  } else if (cur_lex->GetType() == key_word) {
    HandleKeyWords();
  } else
    throw Exception("Wrong line", cur_lex->GetString(),
                    cur_lex->GetLineNum());
}

void SyntaxAnalyzer::Expr()
{
#if defined(PRINT_MODE)
  printf("<Expr>->");
#endif
  Ari1();
  if (!strcmp(">", cur_lex->GetString()) ||
      !strcmp("<", cur_lex->GetString()) ||
      !strcmp("<>", cur_lex->GetString()) ||
      !strcmp(">=", cur_lex->GetString()) ||
      !strcmp("<=", cur_lex->GetString()) ||
      !strcmp("==", cur_lex->GetString())) {
    NextNotNull();
    Ari1();
  }
}

void SyntaxAnalyzer::Ari1()
{
#if defined(PRINT_MODE)
  printf("<Ari1>->");
#endif
  Ari2();
  while (!strcmp("+", cur_lex->GetString()) ||
         !strcmp("-", cur_lex->GetString()) ||
         !strcmp("|", cur_lex->GetString())) {
    NextNotNull();
    Ari2();
  }
}

void SyntaxAnalyzer::Ari2()
{
#if defined(PRINT_MODE)
  printf("<Ari2>->");
#endif
  Ari3();
  while (!strcmp("*", cur_lex->GetString()) ||
         !strcmp("/", cur_lex->GetString()) ||
         !strcmp("&", cur_lex->GetString())) {
    NextNotNull();
    Ari3();
  }
}

void SyntaxAnalyzer::Ari3()
{
#if defined(PRINT_MODE)
  printf("<Ari3>->");
#endif
  if (cur_lex->GetString()[0] == '$') {
    Variable();
  } else if (cur_lex->GetString()[0] == '?') {
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
  } else if (cur_lex->GetType() == number) {
    NextNotNull();
  } else if (!strcmp("!", cur_lex->GetString())) {
    NextNotNull();
    Ari3();
  } else if (!strcmp("(", cur_lex->GetString())) {
    NextNotNull();
    Expr();
    CheckLexeme(")", "Missed ')' after expression");
  } else
    throw Exception("Wrong arithmetic member", cur_lex->GetString(),
                    cur_lex->GetLineNum());
}

void SyntaxAnalyzer::PrintArg()
{
#if defined(PRINT_MODE)
  printf("<PrintArg>->");
#endif
  if (cur_lex->GetType() == const_string) {
    NextNotNull();
  } else
    Expr();
}

void SyntaxAnalyzer::Variable()
{
#if defined(PRINT_MODE)
  printf("<Variable>->");
#endif
  if (cur_lex->GetString()[0] == '$') {
    NextNotNull();
    if (!strcmp("[", cur_lex->GetString())) {
      NextNotNull();
      Expr();
      CheckLexeme("]", "Missed ']' after array");
    }
  } else
    throw Exception("Wrong variable", cur_lex->GetString(),
                    cur_lex->GetLineNum());
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

void PrintListLexeme(List<Lexeme> &list)
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

void HandleNextLexeme(List<Lexeme> &list, Lexeme *lex)
{
  if (lex) {
    list.Append(*lex);
    delete lex;
  }
}

int main(int argc, char **argv) {
  char current_sym;
  FILE *file_in;
  Automatic machine;
  SyntaxAnalyzer *syntax;
  Lexeme *lex = NULL;
  List<Lexeme> list_lex;

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
  syntax->Check();
  return 0;
}
