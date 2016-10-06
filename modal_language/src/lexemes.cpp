#include "lexemes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
