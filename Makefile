include make.inc

SRCS = $(wildcard $(foreach EXT,$(SRC_EXTS),$(patsubst %,%/*$(EXT),.)))
HEADERS = $(wildcard $(patsubst %,%/*.h,.))


all: $(LIB).so

$(LIB).so: $(SRCS:.cpp=.o)
	$(CXX) -shared -o $@ $^

$(LIB).a: $(SRCS:.cpp=.o)
	$(AR) rcsv $@ $^

%.o: %.cpp %.h Makefile
	$(CXX) $< -c -o $@ $(CXXFLAGS) -fPIC -DPIC

test: all
	@$(MAKE) -C $(TEST_DIR) --no-print-directory

clean:
	@$(MAKE) -C $(TEST_DIR) clean --no-print-directory
	$(RM) -f *.o *~ *.a *.so

.PHONY: clean test
