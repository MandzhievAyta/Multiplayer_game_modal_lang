#ifndef LEXANALYZER_H_SENTRY
#define LEXANALYZER_H_SENTRY
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
#endif
