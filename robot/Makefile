SRCMODULES = lexemes.cpp lexanalyzer.cpp ipnclasses.cpp\
             syntanalyzer.cpp interpreter.cpp
OBJMODULES = $(SRCMODULES:.cpp=.o)
CXX = g++
CXXFLAGS = -Wall -g
PARAMETRS = "0 3333 stupid"

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

runrobot: robot.cpp $(OBJMODULES)
	$(CXX) $(CXXFLAGS) $^ -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CXX) -MM $^ > $@

run: runrobot
	./runrobot $(PARAMETRS)

clean: 
	rm -f *.o runrobot
