# 'Hello world'
DEFINES = -DOLD_STD_CPP
STD=c++03
#STD=c++11
CXXFLAGS = -std=$(STD) -Os $(DEFINES)
OUTSTREAM = outstream.o
LDFLAGS = outstream.o
TARGET = hello

$(TARGET): $(TARGET).o $(OUTSTREAM)
	$(CXX) -o $@ $< $(OUTSTREAM)

clean:
	rm $(TARGET).o
	rm $(TARGET)
