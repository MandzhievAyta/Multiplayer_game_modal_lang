#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ipnclasses.h"
#include "syntanalyzer.h"
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

