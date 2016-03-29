# building and testing outstreams
CXXFLAGS = -std=c++11 -g
OUTSTREAM = outstream.o
INSTREAM = instream.o
LDFLAGS = outstream.o
TESTS = testout speedtest testins
all: $(TESTS)

testout: testout.o $(OUTSTREAM)
	$(CXX) -o $@ $< $(OUTSTREAM)
	
test_out: testout
	./testout > test.tmp
	diff test.tmp test.results

speed: speedtest
	./speedtest
	
test_in: testins
	./testins > test.tmp
	diff test.tmp read.results
	
tests: test_out test_in

speedtest: speedtest.o $(OUTSTREAM)
	$(CXX) -o $@ $< $(OUTSTREAM)

testins: testins.o  $(INSTREAM) $(OUTSTREAM)
	$(CXX) -o $@ $< $(INSTREAM) $(OUTSTREAM)

conversions: conversions.o  $(INSTREAM) $(OUTSTREAM)
	$(CXX) -o $@ $< $(INSTREAM) $(OUTSTREAM)


clean:
	rm *.o
	rm $(TESTS)
