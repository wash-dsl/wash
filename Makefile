CXX=g++

serial: 
	$(CXX) -o serial wash_main.cpp wash_mockapi.cpp

vector_test:
	$(CXX) -o vector_test vector_test.cpp