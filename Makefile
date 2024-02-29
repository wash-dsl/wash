# COMPILERS (MPI)
OMPI_CXX = clang++
OMPI_CC  = clang

# COMPILERS (DEFAULT) + C++ STD VERSION
CXX    = clang++ -std=c++17
MPICXX = mpicxx -std=c++17

# DEFAULT COMPILER FLAGS
DEBUG_FLAGS = -g
CXXFLAGS    = -fopenmp

# WASH API LIBRARY PUBLIC HEADERS 
WASH_DIR   = include/
WASH_FLAGS = -I$(WASH_DIR)

# WASH API IMPLEMENTATION SOURCES
WASH_WSER   = $(wildcard src/impl/wser/*.cpp)   # Serial API Implementation
WASH_WISB   = $(wildcard src/impl/wisb/*.cpp)   # WISB   API Implementation
WASH_WEST   = $(wildcard src/impl/west/*.cpp)   # WEST   API Implementation
WASH_CSTONE = $(wildcard src/impl/cstone/*.cpp) # CSTONE API Implementation
WASH_WONE   = $(wildcard src/impl/wone/*.cpp)   # WONE   API Implementation

# UTILITY IMPLEMENTATIONS
WASH_IO    = $(wildcard src/io/*.cpp)
WASH_INPUT = $(wildcard src/input/*.cpp)

# EXAMPLES AND MINIAPP SOURCES
FSIM2_SRCS     = $(wildcard src/examples/ca_fluid_sim/*.cpp)     # Code Adventures 2D simulation
FSIM3_SRCS     = $(wildcard src/examples/3d_fluid_sim/*.cpp)     # Code Adventures 3D simulation
SEDOV_SRCS     = $(wildcard src/examples/sedov_blast_wave/*.cpp) # Sedov Blast Wave Mini-App
SEDOV_SOL_SRCS = $(wildcard src/examples/sedov_solution/*.cpp)   # Analytical soln to Sedov from SPH-EXA

# CORNERSTONE (OCTREE) DEPENDENCY
CSTONE_DIR   = src/cornerstone-octree/include
CSTONE_FLAGS = -I$(CSTONE_DIR)

# ARGPARSE (ARGUMENT PARSING) DEPENDENCY
ARGPARSE_DIR   = argparse/include
ARGPARSE_FLAGS = -I$(ARGPARSE_DIR)

# HDF5 (FILE IO) DEPENDENCY
ifdef HDF5_ROOT # Allows a different env var to specify the HDF5 source location
	_HDF5_ROOT = $(HDF5_ROOT)
endif
ifdef HDF5_DIR
	_HDF5_ROOT = $(HDF5_DIR)
endif
ifdef HDF5ROOT
	_HDF5_ROOT = $(HDF5ROOT)
endif

# If the HDF5 source if found, add the library to the compilation process
ifneq ($(_HDF5_ROOT),)
	HDF5_LIBS    = -L$(_HDF5_ROOT)/lib
	HDF5_INCLUDE = -I$(_HDF5_ROOT)/include
	HDF5_FLAGS  += -DWASH_HDF5 -lhdf5 $(HDF5_LIBS) $(HDF5_INCLUDE)
endif

# COMPILATION INSTRUCTIONS
TARGET = 
BUILD_PATH = build

# DEBUG OPTIONAL PARAMETERS
ifneq ($(DEBUG),)
	WASH_FLAGS += $(DEBUG_FLAGS)
	CXXFLAGS   += -O0
else
	CXXFLAGS   += -O3 
endif

all: clean $(TARGET)

.PHONY : clean
clean:
	rm -rf $(BUILD_PATH)/*.o
	rm -f $(TESTS) gtest.a gtest_main.a *.o
	rm -rf $(TARGET) *.o

# TODO: Do we still need this
test_io: tests/io_test.cpp $(IO_SRCS) $(API_SRCS)
	$(MPICXX) tests/io_test.cpp $(IO_SRCS) $(API_SRCS) -DDIM=2 $(CFLAGS) $(HDF5_FLAGS) -o $(BUILD_PATH)/test_i2o
	$(MPICXX) tests/io_test.cpp $(IO_SRCS) $(API_SRCS) -DDIM=3 $(CFLAGS) $(HDF5_FLAGS) -o $(BUILD_PATH)/test_i3o

########################################################################################################
#    SEDOV SIMULATIONS 
#

SEDOV_FLAGS = -DDIM=3 -DMAX_FORCES=30 $(WASH_FLAGS) $(HDF5_FLAGS) $(CXXFLAGS)
SEODV_APP_SRCS = $(SEDOV_SRCS) $(WASH_IO)

sedov_wser: $(WASH_WSER) $(SEODV_APP_SRCS)
	$(MPICXX) $(WASH_WSER) $(SEODV_APP_SRCS) -DWASH_WSER $(SEDOV_FLAGS) -o $(BUILD_PATH)/sedov_wser

sedov_wisb: $(WASH_WISB) $(SEODV_APP_SRCS)
	$(MPICXX) $(WASH_WISB) $(SEODV_APP_SRCS) -DWASH_WISB $(SEDOV_FLAGS) -o $(BUILD_PATH)/sedov_wisb

sedov_west: $(WASH_WEST) $(SEODV_APP_SRCS)
#   TODO: USE REWRITE HERE
	$(MPICXX) $(WASH_WEST) $(SEODV_APP_SRCS) -DWASH_WEST $(SEDOV_FLAGS) -o $(BUILD_PATH)/sedov_west

sedov_cstone: $(WASH_CSTONE) $(SEODV_APP_SRCS)
	$(MPICXX) $(WASH_CSTONE) $(SEODV_APP_SRCS) -DWASH_CSTONE $(SEDOV_FLAGS) $(CSTONE_FLAGS) -o $(BUILD_PATH)/sedov_cstone

sedov_wone: $(WASH_WONE) $(SEODV_API_SRCS)
# 	TODO: USE REWRITE HERE
	$(MPICXX) $(WASH_WONE) $(SEODV_API_SRCS) -DWASH_WONE $(SEDOV_FLAGS) $(CSTONE_FLAGS) -o $(BUILD_PATH)/sedov_wone

sedov_sol: $(SEDOV_SOL_SRCS)
	$(CXX) $(SEDOV_SOL_SRCS) $(CFLAGS) -o $(BUILD_PATH)/sedov_sol

########################################################################################################
#    FLUID SIMULATIONS 
#
flsim2: $(IO_SRCS) $(API_SRCS) $(FSIM_SRCS)
	$(MPICXX) $(IO_SRCS) $(API_SRCS) $(FSIM_SRCS) $(WASH_INCLUDE) -DDIM=2 -O3 -fopenmp $(HDF5_FLAGS) -o $(BUILD_PATH)/fluid_sim 

flsim3: $(IO_SRCS) $(API_SRCS) $(FSIM3_SRCS)
	$(MPICXX) $(IO_SRCS) $(API_SRCS) $(FSIM3_SRCS) $(WASH_INCLUDE) -DDIM=3 -O3 -fopenmp $(HDF5_FLAGS) -o $(BUILD_PATH)/flu3d_sim

wisb_flsim2: $(IO_SRCS) $(WISB_SRCS) $(FSIM_SRCS)
	$(MPICXX) $(WISB_SRCS) $(IO_SRCS) $(FSIM_SRCS) -DUSE_WISB -DDIM=2 -O3 -fopenmp $(HDF5_FLAGS) -o $(BUILD_PATH)/wisb_flsim2

wisb_flsim3: $(IO_SRCS) $(WISB_SRCS) $(FSIM3_SRCS)
	$(MPICXX) $(WISB_SRCS) $(IO_SRCS) $(FSIM3_SRCS) -DUSE_WISB -DDIM=3 -O3 -fopenmp $(HDF5_FLAGS) -o $(BUILD_PATH)/wisb_flsim3 

########################################################################################################
#     CLANG TOOLING / PLUGIN STUFF
#

OBJ=build/obj

WS2ST_SRCS =$(wildcard src/ws2st/*.cpp) 
WS2ST_SRCS+=$(wildcard src/ws2st/variables/*.cpp) 
WS2ST_SRCS+=$(wildcard src/ws2st/forces/*.cpp)
WS2ST_SRCS+=$(wildcard src/ws2st/meta/*.cpp)
WS2ST_SRCS+=$(wildcard src/ws2st/halo_exchange/*.cpp)

# WS2ST_OBJS=$(SRCS:$(WS2ST_SRCS)/%.cpp=$(OBJ)/%.o)

# $(OBJ)/%.o : $(WS2ST_SRCS)
#    $(CXX)  -c $<

ws2st: $(WS2ST_SRCS) 
	$(CXX) $(WS2ST_SRCS) $(CLFAGS) $(ARGPARSE_FLAGS) -g -lclang-cpp -lLLVM-16 -o $(BUILD_PATH)/wash
	
$(BUILD_PATH)/wash: ws2st

dsl_flsim2: $(BUILD_PATH)/wash $(FSIM_SRCS)
	$(BUILD_PATH)/wash ./src/examples/ca_fluid_sim --

dsl_flsim3: $(BUILD_PATH)/wash $(FSIM3_SRCS)
	$(BUILD_PATH)/wash ./src/examples/3d_fluid_sim --

dsl_sedov: $(BUILD_PATH)/wash $(SEDOV_SRCS)
	$(BUILD_PATH)/wash ./src/examples/sedov_blast_wave -- -DDIM=3

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
# CXXFLAGS += -g -Wall -Wextra -pthread

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
