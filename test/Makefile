PROGS = \
	   arith_logic_test \
	   exception_test \


BINDIR = bin

# default gtest path is under the top directory.
GTEST ?= googletest
GTESTVER = 1.7.0

BINS = $(addprefix $(BINDIR)/,$(PROGS))

test: $(BINS)
	@for b in $(BINS); do $$b || exit; done


OBJS = $(addsuffix .o,$(PROGS))
DEPS = $(addsuffix .d,$(PROGS))

CXXFLAGS = -Wall -Wextra -pedantic -std=c++11 \
		   -O3 -g \
		   -I../include
CXXFLAGS += -I$(GTEST)/include
LDFLAGS += $(GTEST)/make/gtest_main.a -lpthread

%.o: %.cpp | $(GTEST)
	$(CXX) $< -c -o $@ $(CXXFLAGS) -MP -MMD

define link_rule
$(BINDIR)/$1: $1.o | $(BINDIR) $(GTEST)
	$$(CXX) $$< -o $$@ $$(CXXFLAGS) $$(LDFLAGS)
endef
$(foreach P,$(PROGS),$(eval $(call link_rule,$(P))))


$(BINDIR):
	@mkdir -p $@

$(GTEST):
	@wget https://github.com/google/googletest/archive/release-$(GTESTVER).tar.gz; \
		tar zxvf release-$(GTESTVER).tar.gz && rm release-$(GTESTVER).tar.gz; \
		mv googletest-release-$(GTESTVER) $(GTEST); \
    	cd $(GTEST); cd make; make


clean:
	rm -f $(OBJS) $(DEPS) $(BINS) *~
	rm -rf $(BINDIR)

clean_gtest:
	rm -rf $(GTEST)


.PHONY: clean

-include $(DEPS)

