# print template
STD=c++11
CXXFLAGS = -std=$(STD) -g $(DEFINES)
OUTSTREAM = outstream.o
LDFLAGS = outstream.o
TARGET = print

$(TARGET): $(TARGET).o $(OUTSTREAM)
	$(CXX) -o $@ $< $(OUTSTREAM)

clean:
	rm $(TARGET).o
	rm $(TARGET)
