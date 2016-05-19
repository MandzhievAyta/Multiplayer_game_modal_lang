#include "lexemes.h"
#include "ipnclasses.h"
#include "syntanalyzer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
SyntaxAnalyzer::SyntaxAnalyzer(ListOfLexeme &l1): list(l1)
{
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
#if defined(PRINT_MODE)
    printf("Syntax analyzer correct");
#endif
  }
  catch (SyntException &e) {
    printf(e.ErrorString());
    delete ipn_list;
    throw;
  }
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

void SyntaxAnalyzer::KeyWordPrint()
{
  NextNotNull();
  CheckLexeme("(", "Missed '(' after 'print'");
  ipn_list->Append(new IpnOpenBracket);
  PrintArg();
  while (!strcmp(cur_lex->GetString(), ",")) {
    NextNotNull();
    PrintArg();
  }
  CheckLexeme(")", "Missed ')' after 'print'");
  ipn_list->Append(new IpnPrint);
}

void SyntaxAnalyzer::HandleKeyWords()
{
  if (!strcmp("print", cur_lex->GetString()))
  {
    KeyWordPrint();
  } else
  if (!strcmp("buy", cur_lex->GetString()))
  {
    NextNotNull();
    Expr();
    Expr();
    ipn_list->Append(new IpnBuy);
  } else
  if (!strcmp("sell", cur_lex->GetString()))
  {
    NextNotNull();
    Expr();
    Expr();
    ipn_list->Append(new IpnSell);
  } else
  if (!strcmp("prod", cur_lex->GetString()))
  {
    NextNotNull();
    Expr();
    ipn_list->Append(new IpnProd);
  } else
  if (!strcmp("build", cur_lex->GetString()))
  {
    NextNotNull();
    Expr();
    ipn_list->Append(new IpnBuild);
  } else
  if (!strcmp("endturn", cur_lex->GetString()))
    NextNotNull();
}

void SyntaxAnalyzer::HandleVariable()
{
  Variable();
  CheckLexeme(":=", "Should be assignment");
  Expr();
  ipn_list->Append(new IpnFunAssign);
}

void SyntaxAnalyzer::HandleIf()
{
  IpnItem *l1, *l2;
  NextNotNull();
  Expr();
  ipn_list->Append(NULL);
  l1 = ipn_list->GetAddr();
  ipn_list->Append(new IpnOpGoFalse);
  Line();
  ipn_list->Append(NULL);
  l2 = ipn_list->GetAddr();
  ipn_list->Append(new IpnOpGo);
  if (!strcmp("else", cur_lex->GetString())) {
    ipn_list->Append(new IpnNoOp);
    l1->p = new IpnLabel(ipn_list->GetAddr());
    NextNotNull();
    Line();
  }
  ipn_list->Append(new IpnNoOp);
  l2->p = new IpnLabel(ipn_list->GetAddr());
  if (!l1->p) {
    l1->p = new IpnLabel(ipn_list->GetAddr());
  }
}

void SyntaxAnalyzer::HandleWhile()
{
  IpnItem *l1, *l2;
  NextNotNull();
  ipn_list->Append(new IpnNoOp);
  l2 = ipn_list->GetAddr();
  CheckLexeme("(", "Missed '(' after 'while'");
  Expr();
  CheckLexeme(")", "Missed ')' after 'while'");
  ipn_list->Append(NULL);
  l1 = ipn_list->GetAddr();
  ipn_list->Append(new IpnOpGoFalse);
  Line();
  ipn_list->Append(new IpnLabel(l2));
  ipn_list->Append(new IpnOpGo());
  ipn_list->Append(new IpnNoOp);
  l1->p = new IpnLabel(ipn_list->GetAddr());
}

void SyntaxAnalyzer::Line()
{
#if defined(PRINT_MODE)
  printf("<Line>->");
#endif
  if ((cur_lex->GetType() == identificator) &&
      (cur_lex->GetString()[0] == '$'))
  {
    HandleVariable();
  } else
  if (!strcmp("if", cur_lex->GetString()))
  {
    HandleIf();
  } else
  if (!strcmp("while", cur_lex->GetString()))
  {
    HandleWhile();
  } else
  if (!strcmp("{", cur_lex->GetString()))
  {
    Block();
  } else
  if (cur_lex->GetType() == key_word)
  {
    HandleKeyWords();
  } else
    throw SyntException("Line can not start with this lexeme",
                        cur_lex->GetString(), cur_lex->GetLineNum());
}

IpnElem *SyntaxAnalyzer::DefineIpnFun()
{
  const char *str_lex = cur_lex->GetString();
  IpnElem *elem;
  if (!strcmp("?my_id", str_lex)) {
    elem = new IpnMyId;
  } else
  if (!strcmp("?turn", str_lex)) {
    elem = new IpnTurn;
  } else
  if (!strcmp("?players", str_lex)) {
    elem = new IpnPlayers;
  } else
  if (!strcmp("?active_players", str_lex)) {
    elem = new IpnActivePlayers;
  } else
  if (!strcmp("?supply", str_lex)) {
    elem = new IpnSupply;
  } else
  if (!strcmp("?raw_price", str_lex)) {
    elem = new IpnRawPrice;
  } else
  if (!strcmp("?demand", str_lex)) {
    elem = new IpnDemand;
  } else
  if (!strcmp("?production_price", str_lex)) {
    elem = new IpnProductionPrice;
  } else
  if (!strcmp("?money", str_lex)) {
    elem = new IpnMoney;
  } else
  if (!strcmp("?raw", str_lex)) {
    elem = new IpnRaw;
  } else
  if (!strcmp("?production", str_lex)) {
    elem = new IpnProduction;
  } else
  if (!strcmp("?factories", str_lex)) {
    elem = new IpnFactories;
  } else
  if (!strcmp("?manufactured", str_lex)) {
    elem = new IpnManufactured;
  } else
  if (!strcmp("?result_raw_sold", str_lex)) {
    elem = new IpnResultRawSold;
  } else
  if (!strcmp("?result_raw_price", str_lex)) {
    elem = new IpnResultRawPrice;
  } else
  if (!strcmp("?result_prod_bought", str_lex)) {
    elem = new IpnResultProdBought;
  } else
  if (!strcmp("?result_prod_price", str_lex)) {
    elem = new IpnResultProdPrice;
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
    str_lex = cur_lex->GetString();
    ipn_list->Append(elem);
  }
}

void SyntaxAnalyzer::Ari3()
{
  IpnElem *elem;
#if defined(PRINT_MODE)
  printf("<Ari3>->");
#endif
  if (cur_lex->GetString()[0] == '$') {
    Variable();
    ipn_list->Append(new IpnTakeValue);
  } else
  if (cur_lex->GetString()[0] == '?') {
    elem = DefineIpnFun();
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
    ipn_list->Append(elem);
  } else
  if (cur_lex->GetType() == number) {
    ipn_list->Append(new IpnInt(atoi(cur_lex->GetString())));
    NextNotNull();
  } else
  if (!strcmp("!", cur_lex->GetString())) {
    NextNotNull();
    Ari3();
    ipn_list->Append(new IpnFunLogNot);
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
  NextNotNull();
  while (!strcmp("[", cur_lex->GetString())) {
    NextNotNull();
    Expr();
    CheckLexeme("]", "Missed ']' after array");
    ipn_list->Append(new IpnFunAddDimension);
  }
}

