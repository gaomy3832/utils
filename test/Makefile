include ../make.inc

all: main
	@echo "Run test ..."
	@./main
	@echo "Should further use valgrind to check memory leakage."

main: main.o ../$(LIB).so
	$(CXX) $^ -o $@ $(CXXFLAGS)

%.o: %.cpp
	$(CXX) $< -c -o $@ $(CXXFLAGS) -I.. -fPIC -DPIC -MP -MMD

clean:
	$(RM) -f *.o *.d *~ main

.PHONY: all clean

-include main.d
