CC=g++
CFLAGS=-I/opt/exp_software/nvidia/cuda-5.5/include/ -I/home/sstoyanov/env/include -std=c++11 #-Wall
LFLAGS=-lOpenCL -lm -L/home/sstoyanov/env/lib64 -L/home/sstoyanov/env/lib -lopencv_highgui

all: opencl_002

opencl_002: main.o OpenCLass.o
	$(CC) main.o OpenCLass.o -o opencl_002 $(LFLAGS)

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@



clean:
	rm -rf *o hello
