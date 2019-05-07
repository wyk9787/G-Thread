CPP := clang++
CPPFLAGS := -std=c++11 -g -Wall -lpthread -O0
SRCS := $(wildcard *.cc)

all: $(SRCS:.cc=)

clean:
	    rm -rf $(SRCS:.cc=)

%: %.cc
	    $(CPP) $(CPPFLAGS) -o $@ $^
