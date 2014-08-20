CXXFLAGS = -O3 -flto -Wall

LIB = libcommutils

SRC_EXTS = .cpp .c
SRCS = $(wildcard $(foreach EXT,$(SRC_EXTS),$(patsubst %,%/*$(EXT),.)))
HEADERS = $(wildcard $(patsubst %,%/*.h,.))


all: $(LIB).so

$(LIB).so: $(SRCS:.cpp=.o)
	$(CXX) -shared -o $@ $^

$(LIB).a: $(SRCS:.cpp=.o)
	$(AR) rcsv $@ $^

%.o: %.cpp %.h Makefile
	$(CXX) $< -c -o $@ $(CXXFLAGS) -fPIC -DPIC

clean:
	$(RM) -f *.o *~ *.a *.so


