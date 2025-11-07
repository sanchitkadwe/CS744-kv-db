CXX = g++
CXXFLAGS = -std=c++17 -O2 -Iinclude -Wall
LDFLAGS = -lmysqlclient -pthread

SRCDIR = src
BINDIR = bin

all: dirs $(BINDIR)/kv_server $(BINDIR)/loadgen

dirs:
	mkdir -p $(BINDIR)

$(BINDIR)/kv_server: $(SRCDIR)/cache.cpp $(SRCDIR)/db.cpp $(SRCDIR)/threadpool.cpp $(SRCDIR)/server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(SRCDIR)/cache.cpp $(SRCDIR)/db.cpp $(SRCDIR)/threadpool.cpp $(SRCDIR)/server.cpp $(LDFLAGS)

$(BINDIR)/loadgen: $(SRCDIR)/loadgen.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(SRCDIR)/loadgen.cpp -pthread

clean:
	rm -rf bin
