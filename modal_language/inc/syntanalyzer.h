#ifndef SYNTANALYZER_H_SENTRY
#define SYNTANALYZER_H_SENTRY
#include "lexemes.h"
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
#endif
