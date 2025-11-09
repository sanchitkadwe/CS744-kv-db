CXX = g++
CXXFLAGS = -std=c++17 -O2 -Iinclude -Wall
LDFLAGS_SERVER = -lmysqlclient -pthread
LDFLAGS_LOADGEN = -lcurl -pthread

SRCDIR = src
BINDIR = bin

all: dirs $(BINDIR)/kv_server $(BINDIR)/loadgen

dirs:
	mkdir -p $(BINDIR)

$(BINDIR)/kv_server: $(SRCDIR)/cache.cpp $(SRCDIR)/db.cpp $(SRCDIR)/threadpool.cpp $(SRCDIR)/server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS_SERVER)

$(BINDIR)/loadgen: $(SRCDIR)/load_generator.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS_LOADGEN)

clean:
	rm -rf $(BINDIR)
