CXX=clang++ -std=c++17
CFLAGS=-g

# SRCS = $(wildcard *.cpp)
# OBJS = $(patsubst %.cpp,%.o,$(SRCS))
# TARGET = wash_serial

# all: $(TARGET)

# $(TARGET): $(OBJS)
# 	$(CXX) $(CLFAGS) -o $@ $^

# %.o: %.cpp
# 	$(CXX) $(CFLAGS) -c $<

# clean:
# 	rm -rf $(TARGET) *.o

serial: wash_main.cpp wash_mockapi.cpp
	$(CXX) wash_main.cpp wash_mockapi.cpp $(CFLAGS) -o serial

vector_test: vector_test.cpp
	$(CXX) vector_test.cpp $(CFLAGS) -o vector_test