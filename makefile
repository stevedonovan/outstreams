# building and testing outstreams
CXXFLAGS = -std=c++11
LDFLAGS = outstream.o
TESTS = testout speedtest
all: $(TESTS)

testout: testout.o outstream.o
	g++ -o $@ $< $(LDFLAGS)
	
test: testout
	./testout > test.tmp
	diff test.tmp test.results

speed: speedtest
	./speedtest

speedtest: speedtest.o outstream.o
	g++ -o $@ $< $(LDFLAGS)
	
clean:
	rm *.o
	rm $(TESTS)