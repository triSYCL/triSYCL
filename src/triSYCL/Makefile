TARGET = triSYCL_tool

CXXFLAGS = -Wall -std=c++1z
CPPFLAGS = -I ../include

# Installing the libboost-all-dev package may help for this library
LDLIBS = -lboost_program_options -lrt -lpthread

all: $(TARGET)

clean:

	$(RM) $(TARGET)
