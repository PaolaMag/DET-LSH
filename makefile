# Makefile

# Variables
EIGEN_PATH = .\eigen-3.4.0

# Compilation rule
all: main

main: main.cpp LSH.cpp encoding.cpp
	g++ -I$(EIGEN_PATH) main.cpp LSH.cpp encoding.cpp -o main
