#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexemes.h"
#include "ipnclasses.h"
#include "lexanalyzer.h"
#include "syntanalyzer.h"
#include "interpreter.h"
int main(int argc, char **argv)
{
  if (argc != 2) {
    printf("Usage: ./runprogram input_file_name\n");
    return 1;
  }
  Interpreter program;
  program.Run(argv[1]);
  return 0;
}
