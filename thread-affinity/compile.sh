#!/bin/bash

g++ --std=c++20 -O3 -lpthread -lbenchmark -o os os.cpp
g++ --std=c++20 -O3 -lpthread -lbenchmark -o affinity affinity.cpp
