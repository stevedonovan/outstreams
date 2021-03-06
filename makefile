# building and testing outstreams
CXXFLAGS = -std=c++11 -g
OUTSTREAM = outstream.o
INSTREAM = instream.o
LDFLAGS = outstream.o
TESTS = testout speedtest testins
all: $(TESTS) conversions reader-lineinfo

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

$(INSTREAM): instream.cpp instream.h

$(OUTSTREAM): outstream.cpp outstream.h

speedtest: speedtest.o $(OUTSTREAM)
	$(CXX) -o $@ $< $(OUTSTREAM)

testins: testins.o  $(INSTREAM) $(OUTSTREAM)
	$(CXX) -o $@ $< $(INSTREAM) $(OUTSTREAM)

conversions: conversions.o  $(INSTREAM) $(OUTSTREAM)
	$(CXX) -o $@ $< $(INSTREAM) $(OUTSTREAM)

reader-lineinfo: reader-lineinfo.o  $(INSTREAM) $(OUTSTREAM)
	$(CXX) -o $@ $< $(INSTREAM) $(OUTSTREAM)

testlog: testlog.o logger.o  $(OUTSTREAM)
	$(CXX) -o $@ $<  logger.o $(OUTSTREAM) -llog4cpp

clean:
	rm *.o
	rm $(TESTS)
