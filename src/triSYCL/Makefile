TARGETS = gen triSYCL_tool

CXXFLAGS = -Wall -std=c++17
CPPFLAGS = -I../../include

# Installing the libboost-all-dev package may help for this library
LDLIBS = -lboost_program_options -lrt -lpthread

all: $(TARGETS)

clean:

	$(RM) $(TARGETS)
