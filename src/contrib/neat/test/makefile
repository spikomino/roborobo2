
ROOT =../../../../


INCLUDE = -I $(ROOT)/include/contrib/
CCFLAGS = -Wall -std=c++0x -g
LDFLAGS = 
CC      = g++

OBJ = ../gene.o ../genome.o ../link.o ../network.o ../organism.o ../species.o ../genomeAdapted.o ../innovation.o ../neat.o ../nnode.o ../population.o ../trait.o

TARGET = neat-test-network

all=$(TARGET)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CCFLAGS) $(INCLUDE)

$(TARGET): $(OBJ) $(TARGET).o
	$(CC) $(LDFLAGS) -o $@ $^
