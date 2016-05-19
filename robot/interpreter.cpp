#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexemes.h"
#include "ipnclasses.h"
#include "lexanalyzer.h"
#include "syntanalyzer.h"
#include "interpreter.h"
#include "robot.h"

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

bool CheckError(Automatic &machine)
{
  const char *err;
  err = machine.IsError();
  if (err) {
    printf(err);
    return false;
  }
  return true;
}

void HandleNextLexeme(ListOfLexeme &list, Lexeme *lex)
{
  if (lex) {
    list.Append(*lex);
    delete lex;
  }
}

void PrintIpn(ListOfIpnItem *ipn_list)
{
  IpnItem *cur_cmd;
  cur_cmd = ipn_list->GetFirst();
  while (cur_cmd) {
    (*cur_cmd).p->Print();
    printf("  ");
    cur_cmd = cur_cmd->next;
  }
}

ListOfIpnItem **
Interpreter::CreateIpnList(const char *filename, GameContext &context)
{
  int current_sym;
  FILE *file_in;
  Automatic machine;
  SyntaxAnalyzer *syntax = NULL;
  Lexeme *lex = NULL;
  ListOfLexeme list_lex;
  ListOfIpnItem **ipn_list = context.ipn_list;
  ListOfVar *list_var = context.list_var;
  if (!(*ipn_list)) {
    file_in = OpenFile(filename);
    while ((current_sym = fgetc(file_in)) != EOF) {
      lex = machine.FeedChar(current_sym);
      HandleNextLexeme(list_lex, lex);
    }
    lex = machine.FeedChar(' ');
    HandleNextLexeme(list_lex, lex);
#if defined(PRINT_MODE)
    PrintListLexeme(list_lex);
#endif
    if (CheckError(machine)) {
      syntax = new SyntaxAnalyzer(list_lex);
      *ipn_list = syntax->Check(list_var);
    }
  }
  if (syntax)
    delete syntax;
  return ipn_list;
}

void Interpreter::Run(const char *filename, GameContext &context)
{
  ListOfIpnItem **ipn_list;
  IpnElemStack intepr_stack;
  IpnItem *cur_cmd = NULL;

  ipn_list = CreateIpnList(filename, context);
#if defined(PRINT_IPN)
  PrintIpn(*ipn_list);
#endif
  if (*ipn_list)
    cur_cmd = (*ipn_list)->GetFirst();
  try {
    while (cur_cmd) {
      (*cur_cmd).p->Evaluate(&intepr_stack, &cur_cmd, context);
    }
  }
  catch(const IpnEx &e) {
    e.Print();
    throw;
  }
}
