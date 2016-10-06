#include "lexemes.h"
#include "lexanalyzer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

