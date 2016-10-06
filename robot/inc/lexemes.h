#ifndef LEXEME_H_SENTRY
#define LEXEME_H_SENTRY
#include <stddef.h>
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
void PrintListLexeme(ListOfLexeme &);
#endif
