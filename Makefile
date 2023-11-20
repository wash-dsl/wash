CXX=clang++ -std=c++17
CFLAGS=-g

# SRCS = $(wildcard *.cpp)
# OBJS = $(patsubst %.cpp,%.o,$(SRCS))
TARGET = serial vector_test test_io

all: clean $(TARGET)

# $(TARGET): $(OBJS)
# 	$(CXX) $(CLFAGS) -o $@ $^

# %.o: %.cpp
# 	$(CXX) $(CFLAGS) -c $<

.PHONY : clean
clean:
	rm -rf $(TARGET) *.o

serial: wash_main.cpp wash_mockapi.cpp 
	$(CXX) wash_main.cpp wash_mockapi.cpp $(CFLAGS) -o  serial

vector_test: vector_test.cpp
	$(CXX) vector_test.cpp $(CFLAGS) -o vector_test

test_io: ./io/*.cpp wash_mockapi.cpp wash_vector.cpp
	$(CXX) ./io/*.cpp wash_mockapi.cpp wash_vector.cpp $(CFLAGS) -o test_io
