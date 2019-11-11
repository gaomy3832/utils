PROGS = \
	   arith_logic_test \
	   byte_buf_test \
	   chunk_list_test \
	   exception_test \
	   nested_iterator_test \
	   padded_test \
	   random_test \
	   range_chain_test \
	   stream_test \
	   string_utils_test \
	   parallel_test \


BINDIR = bin

GTEST ?= googletest
GTESTVER = 1.10.0
GTEST_BASE = $(GTEST)/googletest
GTEST_BUILD_DIR = build
GTEST_INC_PATH = $(GTEST_BASE)/include
GTEST_LIB_PATH = $(GTEST_BASE)/$(GTEST_BUILD_DIR)/lib
GTEST_LIB = $(GTEST_LIB_PATH)/libgtest_main.a -lgtest

BINS = $(addprefix $(BINDIR)/,$(PROGS))

test: $(BINS)
	@for b in $(BINS); do $$b || exit; done


OBJS = $(addsuffix .o,$(PROGS))
DEPS = $(addsuffix .d,$(PROGS))

CXXFLAGS = -Wall -Wextra -pedantic -std=c++11 \
		   -O3 -g \
		   -I../include
CXXFLAGS += -I$(GTEST_INC_PATH) -Wno-sign-compare
LDFLAGS += -L$(GTEST_LIB_PATH)
LDFLAGS += $(GTEST_LIB) -lpthread

%.o: %.cpp | $(GTEST)
	$(CXX) $< -c -o $@ $(CXXFLAGS) -MP -MMD

$(BINDIR)/%: %.o | $(BINDIR) $(GTEST)
	$(CXX) $< -o $@ $(CXXFLAGS) $(LDFLAGS)


$(BINDIR):
	@mkdir -p $@

$(GTEST):
	@wget https://github.com/google/googletest/archive/release-$(GTESTVER).tar.gz; \
		tar zxvf release-$(GTESTVER).tar.gz && rm release-$(GTESTVER).tar.gz; \
		mv googletest-release-$(GTESTVER) $(GTEST); \
		cd $(GTEST_BASE); mkdir $(GTEST_BUILD_DIR) && cd $(GTEST_BUILD_DIR); cmake -DCMAKE_CXX_STANDARD=11 ..; make


clean:
	rm -f $(OBJS) $(DEPS) $(BINS) *~
	rm -rf $(BINDIR)

clean_gtest:
	rm -rf $(GTEST)


.PHONY: test clean clean_gtest

-include $(DEPS)

