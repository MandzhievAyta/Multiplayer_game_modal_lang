OBJPATH = ./obj/
INCPATH = ./inc/
SRCPATH = ./src/
SRCMODULES = $(SRCPATH)lexemes.cpp $(SRCPATH)lexanalyzer.cpp\
             $(SRCPATH)ipnclasses.cpp $(SRCPATH)syntanalyzer.cpp\
             $(SRCPATH)interpreter.cpp
OBJMODULES = $(OBJPATH)lexemes.o $(OBJPATH)lexanalyzer.o $(OBJPATH)ipnclasses.o\
             $(OBJPATH)syntanalyzer.o $(OBJPATH)interpreter.o
CXX = g++
CXXFLAGS = -Wall -g -I$(INCPATH)

$(OBJPATH)%.o: $(SRCPATH)%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(SRCPATH)main.cpp $(OBJMODULES)
	$(CXX) $(CXXFLAGS) $^ -o interp_modal

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CXX) -I$(INCPATH) -MM $^ > $@

clean:
	rm -f $(OBJPATH)*.o interp_modal deps.mk
