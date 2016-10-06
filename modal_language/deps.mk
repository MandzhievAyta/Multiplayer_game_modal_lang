lexemes.o: src/lexemes.cpp inc/lexemes.h
lexanalyzer.o: src/lexanalyzer.cpp inc/lexemes.h inc/lexanalyzer.h
ipnclasses.o: src/ipnclasses.cpp inc/lexemes.h inc/ipnclasses.h
syntanalyzer.o: src/syntanalyzer.cpp inc/lexemes.h inc/ipnclasses.h \
 inc/syntanalyzer.h inc/lexemes.h
interpreter.o: src/interpreter.cpp inc/lexemes.h inc/ipnclasses.h \
 inc/lexanalyzer.h inc/syntanalyzer.h inc/lexemes.h inc/interpreter.h
