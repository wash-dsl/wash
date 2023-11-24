OMPI_CXX=clang++
OMPI_CC=clang

CXX=clang++ -std=c++17
MPICXX=mpicxx -std=c++17
CFLAGS=-g

IO_SRCS = $(filter-out io/test_io.cpp, $(wildcard io/*.cpp))
FSIM_SRCS = $(wildcard ca_fluid_sim/*.cpp)

# SRCS = $(wildcard *.cpp)
# OBJS = $(patsubst %.cpp,%.o,$(SRCS))
TARGET = serial vector_test test_io fluid_sim

ifndef HDF5ROOT
ifdef HDF5_ROOT
   HDF5ROOT=$(HDF5_ROOT)
endif
ifdef HDF5_DIR
   HDF5ROOT=$(HDF5_DIR)
endif
endif

ifneq ($(HDF5ROOT),)
HDF5LIBS = -L$(HDF5ROOT)/lib
HDF5INCLUDE = -I$(HDF5ROOT)/include
HDF5_FLAGS += -DWASH_HDF5_SUPPORT -lhdf5 $(HDF5LIBS) $(HDF5INCLUDE)
endif

all: clean $(TARGET)

.PHONY : clean
clean:
	rm -rf $(TARGET) *.o

serial: $(IO_SRCS) wash_main.cpp wash_mockapi.cpp wash_vector.cpp
	$(MPICXX) $(IO_SRCS) wash_main.cpp wash_mockapi.cpp wash_vector.cpp $(CFLAGS) $(HDF5_FLAGS) -o serial

vector_test: vector_test.cpp
	$(CXX) vector_test.cpp $(CFLAGS) -o vector_test

test_io: $(IO_SRCS) io/test_io.cpp wash_mockapi.cpp wash_vector.cpp
	$(MPICXX) $(IO_SRCS) io/test_io.cpp wash_mockapi.cpp wash_vector.cpp $(CFLAGS) $(HDF5_FLAGS) -o test_io

fluid_sim: $(FSIM_SRCS) $(IO_SRCS) wash_mockapi.cpp wash_vector.cpp
	$(MPICXX) $(FSIM_SRCS) $(IO_SRCS) wash_mockapi.cpp wash_vector.cpp -O3 -fopenmp $(HDF5_FLAGS) -o fluid_sim 
