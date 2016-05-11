#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexemes.h"
#include "ipnclasses.h"
#include "lexanalyzer.h"
#include "syntanalyzer.h"
#include "interpreter.h"

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

void CheckError(Automatic &machine)
{
  const char *err;
  err = machine.IsError();
  if (err)
    printf(err);
}

void HandleNextLexeme(ListOfLexeme &list, Lexeme *lex)
{
  if (lex) {
    list.Append(*lex);
    delete lex;
  }
}

void Interpreter::Run(const char *filename)
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
  CheckError(machine);
  syntax = new SyntaxAnalyzer(list_lex);
  ipn_list = syntax->Check(list_var);
  cur_cmd = ipn_list->GetFirst();
  while (cur_cmd) {
    (*cur_cmd).p->Evaluate(&intepr_stack, &cur_cmd);
  }
}
