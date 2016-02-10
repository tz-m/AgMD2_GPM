CXX=`root-config --cxx`
CXXFLAGS=`root-config --cflags` -g -I./include -L/usr/lib/CAEN -I/usr/include/CAEN -Wall 
##CXXFLAGS=`root-config --cflags` -pg
LDFLAGS=`root-config --ldflags`
##LDFLAGS=`root-config --ldflags` -pg
LDLIBS=`root-config --glibs` -lAgMD2 -lCAENVME
SRCDIR=./src/
SOURCES=$(SRCDIR)/runGPM.cpp $(SRCDIR)/AgMD2_GPM.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=runGPM

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) -lAgMD2 -o $@ $^ $(LDLIBS) 

.cc.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(CFLAGS) -W -Wall -c $<

clean:
	rm -f ./*~ $(OBJECTS) ./runGPM



