CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -Iinclude -Iinclude/websocket


LDFLAGS = -lssl -lcrypto -lpthread -lcurl


SRCS = main.cpp $(wildcard src/*.cpp)


OBJS = $(SRCS:.cpp=.o)


TARGET = quant


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


clean:
	rm -f $(OBJS) $(TARGET)
