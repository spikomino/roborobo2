
ROOT =../../../../


INCLUDE = -I $(ROOT)/include/contrib/
CCFLAGS = -Wall -std=c++0x -g
LDFLAGS = 
CC      = g++

OBJ = ../gene.o ../genome.o ../link.o ../network.o ../nnode.o

TARGET = test

all=$(TARGET)

all: $(TARGET)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CCFLAGS) $(INCLUDE)

$(TARGET): $(OBJ) test.o
	$(CC) $(LDFLAGS) -o $@ $^
