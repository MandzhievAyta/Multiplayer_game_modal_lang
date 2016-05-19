#ifndef SYNTANALYZER_H_SENTRY
#define SYNTANALYZER_H_SENTRY
#include "lexemes.h"

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
  ListOfIpnItem *ipn_list;
  ListOfVar *list_var;
  void Next() { cur_lex = list.GetNext(); }
  void NextNotNull();
  void CheckLexeme(const char *, const char *);
  void Program();
  void Block();
  void KeyWordPrint();
  void HandleKeyWords();
  void HandleVariable();
  void HandleIf();
  void HandleWhile();
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
  ~SyntaxAnalyzer() {}
  ListOfIpnItem *Check(ListOfVar *a);
};
#endif
