CXX = g++

CXXFLAGS = -Wall -std=c++17

EXECUTABLES = serverM serverC serverRTH serverEEB client

all: $(EXECUTABLES)

client:
	$(CXX) $(CXXFLAGS) -o client client.cpp

serverM:
	$(CXX) $(CXXFLAGS) -o serverM serverM.cpp

serverC:
	$(CXX) $(CXXFLAGS) -o serverC serverC.cpp

serverRTH:
	$(CXX) $(CXXFLAGS) -o serverRTH serverRTH.cpp

serverEEB:
	$(CXX) $(CXXFLAGS) -o serverEEB serverEEB.cpp

clean:
	rm -f $(EXECUTABLES)

.PHONY: all clean
