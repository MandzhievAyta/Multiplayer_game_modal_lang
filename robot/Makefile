OBJPATH = ./obj/
INCPATH = ./inc/
SRCPATH = ./src/
SRCMODULES = $(SRCPATH)robot/lexemes.cpp $(SRCPATH)robot/lexanalyzer.cpp\
             $(SRCPATH)robot/ipnclasses.cpp $(SRCPATH)robot/syntanalyzer.cpp\
             $(SRCPATH)robot/interpreter.cpp
OBJMODULES = $(OBJPATH)lexemes.o $(OBJPATH)lexanalyzer.o $(OBJPATH)ipnclasses.o\
             $(OBJPATH)syntanalyzer.o $(OBJPATH)interpreter.o
CC = gcc
CXX = g++
CFLAGS = -Wall -g -I$(INCPATH)
CXXFLAGS = -Wall -g -I$(INCPATH)

$(OBJPATH)%.o: $(SRCPATH)robot/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(SRCPATH)robot/robot.cpp $(SRCPATH)serv/myserver.c $(OBJMODULES)
	$(CXX) $(CXXFLAGS) $(SRCPATH)robot/robot.cpp $(OBJMODULES) -o robot
	$(CC) $(CFLAGS) $(SRCPATH)serv/myserver.c -o server

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CXX) -I$(INCPATH) -MM $^ > $@

clean:
	rm -f $(OBJPATH)*.o robot server deps.mk
