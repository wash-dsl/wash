OMPI_CXX = clang++
OMPI_CC = clang

CXX = clang++ -std=c++17
MPICXX = mpicxx -std=c++17
NVCC = nvcc -std=c++17 -arch=native -forward-unknown-to-host-compiler -ccbin=mpicxx

NDEBUG_FLAGS = -O3 -march=native -DNDEBUG
DEBUG_FLAGS = -g -DDEBUG
SEDOV_ARGS = -DDIM=3 -DMAX_FORCES=30

# # Need to set this for each machine
# CUDA_DIR = /local/java/cuda-11.4.4
# CUDA_LIBS = -L$(CUDA_DIR)/lib64 -lcudart
# # Need to set this for each machine
# MPI_DIR = /modules/cs402/openmpi
# MPI_INCLUDELIBS = -I$(MPI_DIR)/include -L$(MPI_DIR)/lib64 -lmpi

API_SRCS = $(wildcard src/wash/*.cpp)
# API_CU_SRCS = $(wildcard src/wash/*.cu)
CSTONE_SRCS = \
	src/cornerstone-octree/include/cstone/focus/rebalance_gpu.cu \
	src/cornerstone-octree/include/cstone/focus/source_center_gpu.cu \
	src/cornerstone-octree/include/cstone/halos/gather_halos_gpu.cu \
	src/cornerstone-octree/include/cstone/primitives/primitives_gpu.cu \
	src/cornerstone-octree/include/cstone/sfc/sfc_gpu.cu \
	src/cornerstone-octree/include/cstone/traversal/collisions_gpu.cu \
	src/cornerstone-octree/include/cstone/tree/csarray_gpu.cu \
	src/cornerstone-octree/include/cstone/tree/octree_gpu.cu \
	src/cornerstone-octree/include/cstone/util/reallocate.cu
IO_SRCS = $(wildcard src/io/*.cpp)

SEDOV_SRCS = $(wildcard src/examples/sedov_blast_wave/*.cpp)
# SEDOV_CU_SRCS = $(wildcard src/examples/sedov_blast_wave/*.cu)
FSIM_SRCS = $(wildcard src/examples/ca_fluid_sim/*.cpp)
FSIM3_SRCS = $(wildcard src/examples/3d_fluid_sim/*.cpp)

SEDOV_SOL_SRCS = $(wildcard src/examples/sedov_solution/*.cpp)

# API_OBJECTS = $(API_SRCS:.cpp=.o)
# API_CU_OBJECTS = $(API_CU_SRCS:.cu=.o)
# IO_OBJECTS = $(IO_SRCS:.cpp=.o)
# SEDOV_OBJECTS = $(SEDOV_SRCS:.cpp=.o)
# SEDOV_CU_OBJECTS = $(SEDOV_CU_SRCS:.cu=.o)

CSTONE_DIR = src/cornerstone-octree
CSTONE_FLAGS = -I$(CSTONE_DIR)/include

CUDA_DIR = $(CUDA_HOME)
CUDA_FLAGS = -DUSE_CUDA -I$(CUDA_DIR)/include -L$(CUDA_DIR)/lib64 -lcudart

OPENMP_FLAGS = -fopenmp

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

# SRCS = $(wildcard *.cpp)
# OBJS = $(patsubst %.cpp,%.o,$(SRCS))
TARGET = vector_test test_io fluid_sim sedov_sol sedov
BUILD_PATH = build

all: clean $(TARGET)

.PHONY : clean
clean:
	rm -rf $(BUILD_PATH)/*.o
	rm -f $(TESTS) gtest.a gtest_main.a *.o
	rm -rf $(TARGET) *.o
	rm -rf src/examples/sedov_blast_wave/*.o
	find . -type f -name '*.o' -delete

# Outdated API
# serial: $(IO_SRCS) wash_main.cpp wash_mockapi.cpp wash_vector.cpp
# 	$(MPICXX) $(IO_SRCS) wash_main.cpp wash_mockapi.cpp wash_vector.cpp -DDIM=2 $(DEBUG_FLAGS) $(HDF5_FLAGS) -o serial

test_io: tests/io_test.cpp $(IO_SRCS) $(API_SRCS)
	$(MPICXX) tests/io_test.cpp $(IO_SRCS) $(API_SRCS) -DDIM=2 $(DEBUG_FLAGS) $(HDF5_FLAGS) -o $(BUILD_PATH)/test_i2o
	$(MPICXX) tests/io_test.cpp $(IO_SRCS) $(API_SRCS) -DDIM=3 $(DEBUG_FLAGS) $(HDF5_FLAGS) -o $(BUILD_PATH)/test_i3o

########################################################################################################
#    SEDOV SIMULATIONS 
#

# %.o: %.cpp
# 	$(MPICXX) $(SEDOV_ARGS) $(NDEBUG_FLAGS) -fopenmp $(HDF5_FLAGS) $(CSTONE_FLAGS) -c $< -o $@ $(CUDA_LIBS)

# %.o: %.cu
# 	$(NVCC) $(SEDOV_ARGS) $(NDEBUG_FLAGS) $(NVCCFLAGS) $(HDF5_FLAGS) $(CSTONE_FLAGS) -c $< -o $@

# src/wash/wash.o : src/wash/wash.cu 
# 	$(NVCC) $(MPI_INCLUDELIBS) $(SEDOV_ARGS) $(NDEBUG_FLAGS) $(NVCCFLAGS) $(HDF5_FLAGS) $(CSTONE_FLAGS) -c $< -o $@  

sedov: $(API_SRCS) $(CSTONE_SRCS) $(IO_SRCS) $(SEDOV_SRCS)
	$(NVCC) $(SEDOV_ARGS) $(NDEBUG_FLAGS) \
		$(CSTONE_FLAGS) $(CUDA_FLAGS) $(OPENMP_FLAGS) $(HDF5_FLAGS) \
		$(API_SRCS) $(CSTONE_SRCS) $(IO_SRCS) $(SEDOV_SRCS) \
		-o $(BUILD_PATH)/sedov

sedov_sol: $(SEDOV_SOL_SRCS)
	$(CXX) $(SEDOV_SOL_SRCS) $(DEBUG_FLAGS) -o $(BUILD_PATH)/sedov_sol

########################################################################################################
#    FLUID SIMULATIONS 
#
flsim2: $(IO_SRCS) $(API_SRCS) $(FSIM_SRCS)
	$(MPICXX) $(API_SRCS) $(IO_SRCS) $(FSIM_SRCS) -DDIM=2 $(NDEBUG_FLAGS) $(OPENMP_FLAGS) $(HDF5_FLAGS) -o $(BUILD_PATH)/flsim2

flsim3: $(IO_SRCS) $(API_SRCS) $(FSIM3_SRCS)
	$(MPICXX) $(API_SRCS) $(IO_SRCS) $(FSIM3_SRCS) -DDIM=3 $(NDEBUG_FLAGS) $(OPENMP_FLAGS) $(HDF5_FLAGS) -o $(BUILD_PATH)/flsim3 

########################################################################################################
#     PLUGIN STUFF
#
inspect: src/gen/inspect.cpp
	$(CXX) src/gen/inspect.cpp $(DEBUG_FLAGS) -lclang -o $(BUILD_PATH)/inspect

# findwashfn: src/gen/finder_tool.cpp src/gen/finder.cpp
# 	$(CXX) src/gen/finder_tool.cpp src/gen/finder.cpp $(DEBUG_FLAGS) -lclang-cpp -lLLVM-16 -o $(BUILD_PATH)/findwashfn

# findwashfn.so: src/gen/finder_plugin.cpp src/gen/finder.cpp
# 	$(CXX) src/gen/finder_plugin.cpp src/gen/finder.cpp $(DEBUG_FLAGS) -shared -fPIC -lclang-cpp -lLLVM-16 -o $(BUILD_PATH)/lib/findwashfn.so

# plugin_fsim: $(FSIM_SRCS) findwashfn.so
# 	$(CXX) -fplugin=$(BUILD_PATH)/lib/findwashfn.so $(FSIM_SRCS) -DDIM=2 $(NDEBUG_FLAGS) -o $(BUILD_PATH)/fluid_sim

kernels: src/gen/kernels.cpp
	$(CXX) src/gen/kernels.cpp $(DEBUG_FLAGS) -lclang-cpp -lLLVM-16 -o $(BUILD_PATH)/kernels
	$(CXX) src/gen/kernels.cpp $(DEBUG_FLAGS) -lclang-cpp -lLLVM-16 -shared -fPIC -DPLUGIN -o $(BUILD_PATH)/lib/kernels.so

kernel_plugin: $(FSIM_SRCS)
	$(CXX) -fplugin=$(BUILD_PATH)/lib/kernels.so $(FSIM_SRCS) -DDIM=2 $(NDEBUG_FLAGS) -o $(BUILD_PATH)/fluid_sim

########################################################################################################

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

run_tests : vector_test particle_test

vector_test : tests/vector_test.cpp gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $(BUILD_PATH)/$@
	$(BUILD_PATH)/$@

# To compile wash.hpp looks like we need the IO_SRCS as well due to the get_io function in io.cpp
particle_test : $(IO_SRCS) $(API_SRCS) tests/particle_test.cpp gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -DDIM=3 -lpthread $^ -o $(BUILD_PATH)/$@
	$(BUILD_PATH)/$@
