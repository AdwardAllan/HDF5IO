
#NVCC = /usr/local/gcc12/bin/g++
#LIBRARIES = -std=c++20 -fmodules-ts -L/usr/local/openmpi-gnu/lib  -L/usr/local/hdf5-parallel-gnu/lib -lhdf5 -lmpi -lgsl -lgslcblas -lm -fext-numeric-literals
#INCLUDES = -I/usr/local/openmpi-gnu/include -I/usr/local/hdf5-parallel-gnu/include
# /usr/local/gcc12/bin/g++ -std=c++20 -fmodules-ts -xc++-system-header cstdlib 
# /usr/local/gcc12/bin/g++ -std=c++20 -fmodules-ts -xc++-system-header iostream
NVCC = g++
LIBRARIES = -std=c++20 -fmodules-ts -L/opt/rh/gcc-toolset-12/root/usr/lib  -lhdf5 -lmpi -lgsl -lgslcblas -lm
INCLUDES = -I/opt/rh/gcc-toolset-12/root/usr/include

HDF5: LXJ_HDF5.c Main.c
	 $(NVCC) $(INCLUDES) $(LIBRARIES) $^ -o $@


gsl_0: ../Bsplines/gsl_0.c
	$(NVCC) $(NVCC_FLAGS) $^ -o $@ $(INCLUDES) $(LIBRARIES)

Simple0: Simple0.c
	$(NVCC) $(NVCC_FLAGS) $^ -o $@ $(INCLUDES) $(LIBRARIES)

Simple1: Simple1.c
	$(NVCC) $(NVCC_FLAGS) $^ -o $@ $(INCLUDES) $(LIBRARIES)

Simple2: Simple2.c
	$(NVCC) $(NVCC_FLAGS) $^ -o $@ $(INCLUDES) $(LIBRARIES)


Simple3: Simple3.c
	$(NVCC) $(NVCC_FLAGS) $^ -o $@ $(INCLUDES) $(LIBRARIES)

Simple4: Simple4.c
	$(NVCC) $(NVCC_FLAGS) $^ -o $@ $(INCLUDES) $(LIBRARIES)


clean:
	rm -f *.o *.exe
