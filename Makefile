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

BUILD_PATH = build

all: clean $(TARGET)

.PHONY : clean
clean:
	rm -rf $(BUILD_PATH)/*.o
	rm -f $(TESTS) gtest.a gtest_main.a *.o
	rm -rf $(TARGET) *.o

serial: $(IO_SRCS) wash_main.cpp wash_mockapi.cpp wash_vector.cpp
	$(MPICXX) $(IO_SRCS) wash_main.cpp wash_mockapi.cpp wash_vector.cpp -DDIM=2 $(CFLAGS) $(HDF5_FLAGS) -o serial

test_io: ./io/*.cpp wash_mockapi.cpp wash_vector.cpp
	$(MPICXX) ./io/*.cpp wash_mockapi.cpp wash_vector.cpp -DDIM=2 $(CFLAGS) $(HDF5_FLAGS) -o $(BUILD_PATH)/test_io
  
fluid_sim: $(FSIM_SRCS) $(IO_SRCS) wash_mockapi.cpp wash_vector.cpp
	$(MPICXX) $(FSIM_SRCS) $(IO_SRCS) wash_mockapi.cpp wash_vector.cpp -DDIM=2 -O3 -fopenmp $(HDF5_FLAGS) -o fluid_sim 

# GTEST ---------------
# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = googletest/googletest

# Where to find user code.
USER_DIR = .

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TESTS = vector_test_2

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# House-keeping build targets.
#-------------------

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

# Builds a sample test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

vector_test : tests/vector_test.cpp gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $(BUILD_PATH)/$@
