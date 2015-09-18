CXXFLAGS = -g -O0 -fno-inline -Wall -std=c++11
#CXXFLAGS = -O3 -flto -Wall -std=c++11

all: test run

run: test
	@echo "Run test ..."
	@./test
	@echo "Should further use valgrind to check memory leakage: valgrind --leak-check=yes ./test"

test: test.o
	$(CXX) $^ -o $@ $(CXXFLAGS)

%.o: %.cpp
	$(CXX) $< -c -o $@ $(CXXFLAGS) -I.. -fPIC -DPIC -MP -MMD

clean:
	$(RM) -f *.o *.d *~ test

.PHONY: all clean

-include test.d
