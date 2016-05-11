SRCMODULES = lexemes.cpp lexanalyzer.cpp ipnclasses.cpp\
             syntanalyzer.cpp interpreter.cpp
OBJMODULES = $(SRCMODULES:.cpp=.o)
CXX = g++
CXXFLAGS = -Wall -g

%.o: %.cpp %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

runprogram: main.cpp $(OBJMODULES)
	$(CXX) $(CXXFLAGS) $^ -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CXX) -MM $^ > $@

run: runprogram
	./runprogram

clean: 
	rm -f *.o runprogram
